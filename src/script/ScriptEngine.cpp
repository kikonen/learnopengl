#include "ScriptEngine.h"

#include <map>

#include <ranges>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/thread.h"

#include "util/util.h"
#include "util/file.h"

#include "engine/PrepareContext.h"

#include "script/CommandEngine.h"
#include "script/NodeCommandAPI.h"
#include "script/UtilAPI.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "user_type/LuaUtil.h"
#include "user_type/LuaGlm.h"
//#include "user_type/LuaVec3.h"
#include "user_type/LuaNode.h"
#include "user_type/LuaCommand.h"


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

    ScriptEngine::~ScriptEngine()
    {
    }

    void ScriptEngine::clear()
    {
        ASSERT_WT();

        std::lock_guard lock(m_lock);

        // TODO KI clear scriptlets from Lua
        if (m_commandEngine) {
            for (auto& [nodeId, functions] : m_nodeFunctions) {
                for (auto& [scriptId, fnName] : functions) {
                    unregisterFunction(fnName);
                }
            }

            for (auto& apiName : m_nodeCommandApis) {

            }

            sol::table nodes = getState()["nodes"];
            for (auto& node : nodes) {

            }
        }

        m_nodeCommandApis.clear();
        m_nodeFunctions.clear();
        //m_nodeScripts.clear();

        m_scripts.clear();
    }

    void ScriptEngine::shutdown()
    {
        ASSERT_WT();

        clear();
    }

    void ScriptEngine::prepare(
        const PrepareContext& ctx,
        CommandEngine* commandEngine)
    {
        ASSERT_WT();

        const auto& assets = ctx.m_assets;

        m_commandEngine = commandEngine;

        //m_utilApi = std::make_unique<UtilAPI>();

        m_state = std::make_unique<sol::state>();

        auto& lua = *m_state;

        lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::math,
            sol::lib::os,
            sol::lib::io,
            sol::lib::coroutine,
            sol::lib::table,
            sol::lib::string);

        // https://github.com/ThePhD/sol2/issues/90
        {
            const std::vector<std::string> paths{
                lua["package"]["path"],
                util::joinPath({ assets.rootDir, assets.sceneDir, "scripts", "?.lua" }),
                util::joinPath({ assets.rootDir, assets.sceneDir, "lib", "?.lua" }),
            };

            const auto notEmpty = [](const std::string& s){ return !s.empty(); };
            auto filtered = paths | std::views::filter(notEmpty);

            const auto packagePath = util::join(
                std::vector<std::string>{ filtered.begin(), filtered.end() },
                ";");

            KI_INFO_OUT(fmt::format("LUA: package.path={}", packagePath));
            lua["package"]["path"] = packagePath;
        }

        registerTypes();

        lua["nodes"] = lua.create_table_with();
    }

    void ScriptEngine::registerTypes()
    {
        // util - static utils
        auto& lua = getState();

        {
            LuaUtil t;
            t.bind(lua);
        }

        {
            LuaCommand t;
            t.bind(lua);
        }
        {
            LuaNode t;
            t.bind(lua);
        }
        {
            LuaGlm t;
            t.bind(lua);
        }
        //{
        //    LuaVec3 t;
        //    t.bind(lua);
        //}
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
        std::lock_guard lock(m_lock);

        const auto& fnName = createNodeFunction(handle, scriptId);

        if (fnName.empty()) return;

        auto it = m_nodeFunctions.find(handle);
        if (it == m_nodeFunctions.end()) {
            std::unordered_map<script::script_id, std::string> fnMap;
            m_nodeFunctions.insert({ handle, fnMap });
            it = m_nodeFunctions.find(handle);
        }
        it->second.insert({ scriptId, fnName });

        m_nodeCommandApis.insert({ handle, std::make_unique<NodeCommandAPI>(m_commandEngine, handle) });

        //if (!m_luaNodes[nodeId]) {
        //    m_luaNodes[nodeId] = m_lua.create_table_with();
        //}
    }

    std::vector<script::script_id> ScriptEngine::getNodeScripts(
        pool::NodeHandle handle)
    {
        std::lock_guard lock(m_lock);

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

        const auto it = m_scripts.find(scriptId);
        if (it == m_scripts.end()) return "";

        const auto& script = it->second;

        // NOTE KI unique wrapperFn for node
        std::string nodeFnName;

        if (handle) {
            // NOTE KI pass context as closure to Node
            // - node, cmd, id
            nodeFnName = fmt::format("fn_{}_{}_{}", handle.toId(), handle.toIndex(), scriptId);

            scriptlet = fmt::format(R"(
function {}(node, cmd, id)
nodes[id] = nodes[id] or {}
local lua_node = nodes[id]
{}
end)", nodeFnName, "{}", script.m_source);
        }
        else {
            // NOTE KI global scriplet
            nodeFnName = fmt::format("fn_global_{}", scriptId);

            scriptlet = fmt::format(R"(
function {}()
{}
end)", nodeFnName, script.m_source);
        }

        auto result = invokeLuaScript(scriptlet);

        return result.valid() ? nodeFnName : "";
    }

    bool ScriptEngine::unregisterFunction(std::string fnName)
    {
        auto& lua = getState();

        if (!lua[fnName].is<sol::function>()) return false;

        std::string undef = fmt::format(R"(
{} = nil)", fnName);

        lua.script(undef);
        KI_INFO_OUT(fmt::format("SCRIPT::UNREGISTER: function={}", fnName));
        return true;
    }

    void ScriptEngine::runGlobalScript(
        script::script_id scriptId)
    {
        std::lock_guard lock(m_lock);

        const auto& it = m_nodeFunctions.find(pool::NodeHandle::NULL_HANDLE);

        if (it == m_nodeFunctions.end()) return;

        if (const auto& fnIt = it->second.find(scriptId);
            fnIt != it->second.end())
        {
            const auto& fnName = fnIt->second;

            invokeLuaFunction([this, &fnName]() {
                sol::protected_function fn(getState()[fnName]);
                return fn();
                });
        }
    }

    void ScriptEngine::runNodeScript(
        Node* node,
        script::script_id scriptId)
    {
        if (!node) return;

        std::lock_guard lock(m_lock);

        const auto& handle = node->toHandle();
        const auto& it = m_nodeFunctions.find(handle);

        if (it == m_nodeFunctions.end()) return;

        if (const auto& fnIt = it->second.find(scriptId);
            fnIt != it->second.end())
        {
            auto& fnName = fnIt->second;
            KI_INFO_OUT(fmt::format("SCRIPT::RUN: function={}", fnName));

            auto* cmdApi = m_nodeCommandApis.find(handle)->second.get();

            invokeLuaFunction([this, node, handle, &fnName, &cmdApi]() {
                sol::protected_function fn((getState())[fnName]);
                return fn(std::ref(node), std::ref(cmdApi), handle.toId());;
            });
        }
    }

    sol::protected_function_result ScriptEngine::execScript(
        const std::string& script)
    {
        std::lock_guard lock(m_lock);
        return invokeLuaScript(script);
    }

    bool ScriptEngine::hasFunction(
        pool::NodeHandle handle,
        std::string_view name)
    {
        std::lock_guard lock(m_lock);

        sol::table luaNode = getState()["nodes"][handle.toId()];

        sol::optional<sol::function> fnPtr = luaNode[name];
        return fnPtr != sol::nullopt;
    }

    void ScriptEngine::invokeNodeFunction(
        Node* node,
        bool self,
        const sol::function& fn,
        const sol::table& args)
    {
        std::lock_guard lock(m_lock);

        invokeLuaFunction([this, node, self, &fn, &args]() {
            sol::table luaNode = getState()["nodes"][node->getId()];
            return self ? fn(luaNode, args) : fn(args);
            });
    }

    void ScriptEngine::emitEvent(
        int listenerId,
        int type,
        const std::string& data)
    {
        std::lock_guard lock(m_lock);

        invokeLuaFunction([this, &type, &data, &listenerId]() {
            sol::table events = getState()["events"];
            sol::protected_function fn(events["emit_raw"]);
            return fn(events, type, data, listenerId);
            });
    }

    sol::protected_function_result ScriptEngine::invokeLuaFunction(
        const std::function<sol::protected_function_result()>& fn)
    {
        try {
            sol::protected_function_result result = fn();
            if (!result.valid()) {
                sol::error err = result;
                std::string what = err.what();
                KI_CRITICAL(fmt::format("SCRIPT::RUNTIME: {}", what));
            }
            return result;
        }
        catch (const std::exception& ex) {
            KI_CRITICAL(fmt::format("SCRIPT::RUNTIME: {}", ex.what()));
        }
        catch (const std::string& ex) {
            KI_CRITICAL(fmt::format("SCRIPT::RUNTIME: {}", ex));
        }
        catch (const char* ex) {
            KI_CRITICAL(fmt::format("SCRIPT::RUNTIME: {}", ex));
        }
        catch (...) {
            KI_CRITICAL("SCRIPT::RUNTIME: UNKNOWN_ERROR");
        }
        return {};
    }

    // https://developercommunity.visualstudio.com/t/exception-block-is-optmized-away-causing-a-crash/253077
    sol::protected_function_result ScriptEngine::invokeLuaScript(
        const std::string& script)
    {
        try {
            KI_INFO_OUT(util::appendLineNumbers(script));
            return getState().safe_script(script);
        }
        catch (const std::exception& ex) {
            KI_CRITICAL(fmt::format("SCRIPT::RUNTIME: {}", ex.what()));
        }
        catch (const std::string& ex) {
            KI_CRITICAL(fmt::format("SCRIPT::RUNTIME: {}", ex));
        }
        catch (const char* ex) {
            KI_CRITICAL(fmt::format("SCRIPT::RUNTIME: {}", ex));
        }
        catch (...) {
            KI_CRITICAL("SCRIPT::RUNTIME: UNKNOWN_ERROR");
        }
        return {};
    }
}
