#include "ScriptEngine.h"

#include <map>

#include <ranges>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/util.h"
#include "util/file.h"

#include "engine/PrepareContext.h"

#include "script/CommandEngine.h"
#include "script/NodeCommandAPI.h"
#include "script/UtilAPI.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"


namespace {
    static script::ScriptEngine s_engine;
}

namespace script
{
    ScriptEngine& ScriptEngine::get() noexcept
    {
        return s_engine;
    }

    ScriptEngine::ScriptEngine()
    {
    }

    ScriptEngine::~ScriptEngine() = default;

    void ScriptEngine::prepare(
        const PrepareContext& ctx,
        CommandEngine* commandEngine)
    {
        const auto& assets = ctx.m_assets;

        m_commandEngine = commandEngine;

        m_utilApi = std::make_unique<UtilAPI>();

        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::math,
            sol::lib::os,
            sol::lib::io,
            sol::lib::coroutine,
            sol::lib::string);

        // https://github.com/ThePhD/sol2/issues/90
        {
            const std::vector<std::string> paths{
                m_lua["package"]["path"],
                util::joinPath({ assets.rootDir, assets.sceneDir, "scripts", "?.lua" }),
                util::joinPath({ assets.rootDir, assets.sceneDir, "lib", "?.lua" }),
            };

            const auto notEmpty = [](const std::string& s){ return !s.empty(); };
            auto filtered = paths | std::views::filter(notEmpty);

            const auto packagePath = util::join(
                std::vector<std::string>{ filtered.begin(), filtered.end() },
                ";");

            KI_INFO_OUT(fmt::format("LUA: package.path={}", packagePath));
            m_lua["package"]["path"] = packagePath;
        }

        registerTypes();

        m_lua["nodes"] = m_lua.create_table_with();
        m_luaNodes = m_lua["nodes"];
    }

    void ScriptEngine::registerTypes()
    {
        // util
        {
            m_lua.new_usertype<UtilAPI>("UtilAPI");
            const auto& ut = m_lua["UtilAPI"];
            ut["sid"] = &UtilAPI::lua_sid;
        }

        // NodeCommandAPI
        {
            m_lua.new_usertype<NodeCommandAPI>("NodeCommandAPI");

            const auto& ut = m_lua["NodeCommandAPI"];

            ut["cancel"] = &NodeCommandAPI::lua_cancel;
            ut["wait"] = &NodeCommandAPI::lua_wait;
            ut["sync"] = &NodeCommandAPI::lua_sync;

            ut["move"] = &NodeCommandAPI::lua_move;
            ut["move_spline"] = &NodeCommandAPI::lua_move_spline;
            ut["rotate"] = &NodeCommandAPI::lua_rotate;
            ut["scale"] = &NodeCommandAPI::lua_scale;

            ut["set_text"] = &NodeCommandAPI::lua_set_text;
            ut["set_visible"] = &NodeCommandAPI::lua_set_visible;

            ut["audio_play"] = &NodeCommandAPI::lua_audio_play;
            ut["audio_pause"] = &NodeCommandAPI::lua_audio_pause;
            ut["audio_stop"] = &NodeCommandAPI::lua_audio_stop;

            ut["particle_emit"] = &NodeCommandAPI::lua_particle_emit;
            ut["particle_stop"] = &NodeCommandAPI::lua_particle_stop;

            ut["animation_play"] = &NodeCommandAPI::lua_animation_play;

            ut["resume"] = sol::yielding(&NodeCommandAPI::lua_resume_wrapper);

            ut["start"] = &NodeCommandAPI::lua_start;
        }

        // Node
        {
            m_lua.new_usertype<Node>("Node");

            const auto& ut = m_lua["Node"];

            ut["get_id"] = &Node::lua_getId;
            ut["get_name"] = &Node::lua_getName;

            ut["get_clone_index"] = &Node::lua_getCloneIndex;

            ut["get_pos"] = &Node::lua_getPos;
        }
    }

    script::script_id ScriptEngine::registerScript(std::string_view source)
    {
        std::lock_guard lock(m_lock);

        Script script{ source };
        m_scripts.insert({ script.m_id, script });

        return script.m_id;
    }

    void ScriptEngine::bindNodeScript(
        pool::NodeHandle handle,
        script::script_id scriptId)
    {
        const auto& fnName = createNodeFunction(handle, scriptId);

        if (fnName.empty()) return;

        auto it = m_nodeFunctions.find(handle);
        if (it == m_nodeFunctions.end()) {
            std::unordered_map<script::script_id, std::string> fnMap;
            m_nodeFunctions.insert({ handle, fnMap });
            it = m_nodeFunctions.find(handle);
        }
        it->second.insert({ scriptId, fnName });

        m_commandApis.insert({ handle, std::make_unique<NodeCommandAPI>(this, m_commandEngine, handle) });

        //if (!m_luaNodes[nodeId]) {
        //    m_luaNodes[nodeId] = m_lua.create_table_with();
        //}
    }

    std::vector<script::script_id> ScriptEngine::getNodeScripts(
        pool::NodeHandle handle)
    {
        const auto& it = m_nodeFunctions.find(handle);
        if (it == m_nodeFunctions.end()) return {};

        std::vector<script::script_id> scripts;
        for (const auto& fnIt : it->second) {
            scripts.push_back(fnIt.first);
        }
        return scripts;
    }

    std::string ScriptEngine::createNodeFunction(
        pool::NodeHandle handle,
        script::script_id scriptId)
    {
        std::string scriptlet;

        try {
            std::lock_guard lock(m_lock);

            const auto it = m_scripts.find(scriptId);
            if (it == m_scripts.end()) return "";

            const auto& script = it->second;

            // NOTE KI unique wrapperFn for node
            const std::string nodeFnName = fmt::format("fn_{}_{}_{}", handle.toId(), handle.toIndex(), scriptId);

            // NOTE KI pass context as closure to Node
            // - node, cmd, id
            scriptlet = fmt::format(R"(
function {}(node, util, cmd, id)
nodes[id] = nodes[id] or {}
local luaNode = nodes[id]
{}
end)", nodeFnName, "{}", script.m_source);

            KI_INFO_OUT(util::appendLineNumbers(scriptlet));

            m_lua.script(scriptlet);

            return nodeFnName;
        }
        catch (const std::exception& ex) {
            KI_CRITICAL(fmt::format(
                "SCRIPT: {}\n{}",
                ex.what(), util::appendLineNumbers(scriptlet)));
        }
        catch (const std::string& ex) {
            KI_CRITICAL(fmt::format(
                "SCRIPT: {}\n{}",
                ex, util::appendLineNumbers(scriptlet)));
        }
        catch (const char* ex) {
            KI_CRITICAL(fmt::format(
                "SCRIPT: {}\n{}",
                ex, util::appendLineNumbers(scriptlet)));
        }
        catch (...) {
            KI_CRITICAL(fmt::format(
                "SCRIPT: UNKNOWN_ERROR\n{}",
                util::appendLineNumbers(scriptlet)));
        }

        return {};
    }

    void ScriptEngine::runGlobalScript(
        script::script_id scriptId)
    {
        const auto& it = m_nodeFunctions.find(pool::NodeHandle::NULL_HANDLE);

        if (it == m_nodeFunctions.end()) return;

        if (const auto& fnIt = it->second.find(scriptId);
            fnIt != it->second.end())
        {
            auto& fnName = fnIt->second;
            sol::function fn = m_lua[fnName];
            fn(nullptr, nullptr, nullptr, 0);
        }
    }

    void ScriptEngine::runNodeScript(
        Node* node,
        script::script_id scriptId)
    {
        if (!node) return;

        const auto& handle = node->toHandle();
        const auto& it = m_nodeFunctions.find(handle);

        if (it == m_nodeFunctions.end()) return;

        try {
            if (const auto& fnIt = it->second.find(scriptId);
                fnIt != it->second.end())
            {
                auto& fnName = fnIt->second;
                sol::function fn = m_lua[fnName];

                auto* utilApi = m_utilApi.get();
                auto* cmdApi = m_commandApis.find(handle)->second.get();
                fn(std::ref(node), std::ref(utilApi), std::ref(cmdApi), handle.toId());
            }
        }
        catch (const std::exception& ex) {
            KI_CRITICAL(fmt::format("SCRIPT: {}", ex.what()));
        }
        catch (const std::string& ex) {
            KI_CRITICAL(fmt::format("SCRIPT: {}", ex));
        }
        catch (const char* ex) {
            KI_CRITICAL(fmt::format("SCRIPT: {}", ex));
        }
        catch (...) {
            KI_CRITICAL("SCRIPT: UNKNOWN_ERROR");
        }
    }

    bool ScriptEngine::hasFunction(
        pool::NodeHandle handle,
        std::string_view name)
    {
        sol::table luaNode = m_luaNodes[handle.toId()];

        sol::optional<sol::function> fnPtr = luaNode[name];
        return fnPtr != sol::nullopt;
    }

    void ScriptEngine::invokeFunction(
        Node* node,
        std::string_view name)
    {
        const auto handle = node->toHandle();

        //KI_INFO_OUT(fmt::format("CALL LUA: name={}, id={}, fn={}", node->m_type->m_name, node->getId(), name));
        sol::table luaNode = m_luaNodes[handle.toId()];

        try {
            sol::optional<sol::function> fnPtr = luaNode[name];
            if (fnPtr != sol::nullopt) {
                auto* api = m_commandApis.find(handle)->second.get();
                auto& fn = fnPtr.value();
                fn(std::ref(node), std::ref(api), handle.toId());
            }
        }
        catch (const std::exception& ex) {
            KI_CRITICAL(fmt::format("SCRIPT: {}", ex.what()));
        }
        catch (const std::string& ex) {
            KI_CRITICAL(fmt::format("SCRIPT: {}", ex));
        }
        catch (const char* ex) {
            KI_CRITICAL(fmt::format("SCRIPT: {}", ex));
        }
        catch (...) {
            KI_CRITICAL("SCRIPT: UNKNOWN_ERROR");
        }
    }
}
