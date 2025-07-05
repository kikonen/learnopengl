#include "ScriptSystem.h"

#include <map>

#include <ranges>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/thread.h"

#include "util/util.h"
#include "util/file.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "script/CommandEngine.h"
#include "script/ScriptFile.h"
#include "script/ScriptEntry.h"

#include "model/Node.h"
#include "model/NodeType.h"

#include "mesh/LodMesh.h"

#include "script/api/NodeAPI.h"
#include "script/api/NodeCommandAPI.h"
#include "script/api/UtilAPI.h"
#include "script/api/SceneAPI.h"

#include "binding/LuaUtil.h"
#include "binding/LuaPhysics.h"
#include "binding/LuaScene.h"
#include "binding/LuaNode.h"
#include "binding/LuaNodeCommand.h"
#include "binding/LuaGlm.h"
#include "binding/LuaRayHit.h"
#include "binding/LuaPath.h"

namespace
{
    const static std::string TABLE_CLASSES = "classes";
    const static std::string TABLE_STATES = "states";
    const static std::string TABLE_SCENE = "scene";
    const static std::string TABLE_TMP = "tmp";

    static script::ScriptSystem* s_system{ nullptr };
}

namespace script
{
    void ScriptSystem::init() noexcept
    {
        assert(!s_system);
        s_system = new ScriptSystem();
    }

    void ScriptSystem::release() noexcept
    {
        auto* s = s_system;
        s_system = nullptr;
        delete s;
    }

    ScriptSystem& ScriptSystem::get() noexcept
    {
        assert(s_system);
        return *s_system;
    }
}

namespace script
{
    ScriptSystem::ScriptSystem()
        : m_sceneApi{ std::make_unique<api::SceneAPI>() }
    {
    }

    ScriptSystem::~ScriptSystem()
    {
    }

    void ScriptSystem::clear()
    {
        ASSERT_WT();

        std::lock_guard lock(m_lock);

        // TODO KI clear scriptlets from Lua
        if (m_commandEngine) {
            for (auto& [nodeId, entries] : m_scriptEntries) {
                for (auto& [scriptId, fnName] : entries) {
                    unregisterScriptEntry(fnName);
                }
            }

            for (auto& apiName : m_nodeApis) {
            }

            for (auto& apiName : m_nodeCommandApis) {
            }

            sol::table states = getLua()[TABLE_STATES];
            for (auto& state : states) {

            }
        }

        m_nodeApis.clear();
        m_nodeCommandApis.clear();
        m_scriptEntries.clear();
        //m_nodeScripts.clear();

        m_scripts.clear();
    }

    void ScriptSystem::shutdown()
    {
        ASSERT_WT();

        clear();
    }

    void ScriptSystem::prepare(
        const PrepareContext& ctx,
        CommandEngine* commandEngine)
    {
        ASSERT_WT();

        const auto& assets = ctx.m_assets;

        m_commandEngine = commandEngine;

        //m_utilApi = std::make_unique<UtilAPI>();

        auto& lua = getLua();

        lua.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::jit,
            sol::lib::ffi,
            sol::lib::math,
            sol::lib::os,
            sol::lib::io,
            sol::lib::bit32,
            sol::lib::coroutine,
            sol::lib::table,
            sol::lib::utf8,
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

        lua[TABLE_CLASSES] = lua.create_table_with();
        lua[TABLE_STATES] = lua.create_table_with();
        lua[TABLE_TMP] = lua.create_table_with();

        lua[TABLE_SCENE] = std::ref(m_sceneApi);
    }

    void ScriptSystem::update(const UpdateContext& ctx)
    {
        invokeLuaFunction([this, &ctx]() {
            auto updater = getLua()["Updater"];
            if (!updater.valid()) {
                return sol::protected_function_result{ getLua(), 0, 0, 0, sol::call_status::ok };
            }

            sol::protected_function fn(updater["update"]);
            return fn(updater, ctx.m_clock.elapsedSecs);
            });
    }

    void ScriptSystem::registerTypes()
    {
        // util - static utils
        auto& lua = getLua();

        binding::LuaUtil::bind(lua);
        binding::LuaScene::bind(lua);
        binding::LuaPhysics::bind(lua);
        binding::LuaGlm::bind(lua);
        binding::LuaRayHit::bind(lua);
        binding::LuaPath::bind(lua);
        binding::LuaNodeCommand::bind(lua);
        binding::LuaNode::bind(lua);
    }

    script::script_id ScriptSystem::registerScript(
        script::ScriptFile scriptFile)
    {
        std::lock_guard lock(m_lock);

        scriptFile.m_id = script::ScriptFile::nextId();

        m_scripts.insert({ scriptFile.m_id, scriptFile });

        return scriptFile.m_id;
    }

    void ScriptSystem::bindTypeScript(
        bool global,
        pool::TypeHandle typeHandle,
        script::script_id scriptId)
    {
        std::lock_guard lock(m_lock);

        {
            const auto& signature = getScriptSignature(typeHandle, scriptId);
            if (signature.empty()) return;
        }

        auto it = m_scriptEntries.find(typeHandle);
        if (it == m_scriptEntries.end()) {
            std::unordered_map<script::script_id, script::ScriptEntry> scriptMap;
            m_scriptEntries.insert({ typeHandle, scriptMap });
            it = m_scriptEntries.find(typeHandle);
        }

        {
            const auto& scriptEntry = createScriptEntry(global, typeHandle, scriptId);
            if (scriptEntry.m_valid) {
                it->second.insert({ scriptId, scriptEntry });
            }
        }
    }

    void ScriptSystem::bindNode(
        const Node* node)
    {
        std::lock_guard lock(m_lock);

        auto nodeHandle = node->toHandle();

        {
            m_nodeApis.insert({ nodeHandle, std::make_unique<api::NodeAPI>(nodeHandle) });
            m_nodeCommandApis.insert({ nodeHandle, std::make_unique<api::NodeCommandAPI>(m_commandEngine, nodeHandle) });
        }

        createNodeState(node);
    }

    void ScriptSystem::unbindNode(
        const Node* node)
    {
        std::lock_guard lock(m_lock);

        auto nodeHandle = node->toHandle();

        {
            m_nodeApis.erase(nodeHandle);
            m_nodeCommandApis.erase(nodeHandle);
        }

        deleteNodeState(node);
    }

    void ScriptSystem::createNodeState(
        const Node* node)
    {
        const auto id = node->getId();
        {
            const auto typeId = node->m_typeHandle.toId();
            std::string scriptlet = fmt::format(
                R"(-- {}
states[{}] = classes[{}]:new())",
node->getName(), id, typeId);

            auto result = invokeLuaScript(scriptlet);
            if (!result.valid()) return;
        }

        {
            const auto nodeHandle = node->toHandle();
            auto* nodeApi = m_nodeApis.find(nodeHandle)->second.get();
            auto* cmdApi = m_nodeCommandApis.find(nodeHandle)->second.get();

            sol::table nodeState = getLua()[TABLE_STATES][nodeHandle.toId()];
            nodeState["id"] = node->getId();
            nodeState["cmd"] = std::ref(cmdApi);
            nodeState["node"] = std::ref(nodeApi);
        }
    }

    void ScriptSystem::deleteNodeState(
        const Node* node)
    {
        auto scriptlet = fmt::format(
            "if states[{}] then states[{}]:destroy() end",
            node->getId(), node->getId());

        execScript(scriptlet);

        execScript("Updater:refresh()");
    }

    bool ScriptSystem::hasScriptEntry(
        pool::TypeHandle typeHandle,
        script::script_id scriptId)
    {
        const auto& typeIt = m_scriptEntries.find(typeHandle);
        if (typeIt == m_scriptEntries.end()) return false;

        const auto& scriptMap = typeIt->second;
        const auto& scriptIt = scriptMap.find(scriptId);
        if (scriptIt == scriptMap.end()) return false;
        return scriptIt->second.m_valid;
    }

    std::vector<script::script_id> ScriptSystem::getScriptEntryIds(
        pool::TypeHandle typeHandle)
    {
        std::lock_guard lock(m_lock);

        const auto& it = m_scriptEntries.find(typeHandle);
        if (it == m_scriptEntries.end()) return {};

        std::vector<script::script_id> scripts;
        for (const auto& fnIt : it->second) {
            scripts.push_back(fnIt.first);
        }
        return scripts;
    }

    script::ScriptEntry ScriptSystem::createScriptEntry(
        bool global,
        pool::TypeHandle typeHandle,
        script::script_id scriptId)
    {
        const auto typeId = typeHandle.toId();
        auto* type = typeHandle.toType();

        // NOTE KI unique wrapperFn for node
        const std::string& fnName = getScriptSignature(typeHandle, scriptId);
        if (fnName.empty()) return {};

        const auto it = m_scripts.find(scriptId);
        if (it == m_scripts.end()) return { false };

        const auto& scriptFile = it->second;

        std::string scriptlet;

//        switch (scriptFile.m_type) {
//        case ScriptType::module_file:
//        case ScriptType::class_file:
//        case ScriptType::plain:
//            scriptlet = fmt::format(R"(
//function {}(self)
//{}
//end)", fnName, scriptFile.m_source);
//            break;
//        }

        if (!global) {
            std::string classScriptlet = fmt::format(
            "classes[{}] = classes[{}] or Node:new_class({{ type_id={} }})",
            typeId, typeId, typeId);

            std::string inlineScriptlet;
            if (scriptFile.m_embedded) {
                inlineScriptlet =
R"(local cmd = self.cmd
local node = self.node)";
            }

            // NOTE KI pass context as closure to Node
            // - node, cmd, id
            scriptlet = fmt::format(
R"(-- {} - {}
{}
classes[{}].{} = function(self)
local State = self:class()
{}
{}
end)", fnName, type->getName(), classScriptlet, typeId, fnName, inlineScriptlet, scriptFile.m_source);
        }
        else {
            // NOTE KI global scriplet

            scriptlet = fmt::format(
R"(function {}()
{}
end)", fnName, scriptFile.m_source);
        }

        auto result = invokeLuaScript(scriptlet);
        if (!result.valid()) return { false };

        return { true, ScriptEntryType::function, fnName };
    }

    std::string ScriptSystem::getScriptSignature(
        pool::TypeHandle typeHandle,
        script::script_id scriptId) const
    {
        const auto it = m_scripts.find(scriptId);
        if (it == m_scripts.end()) return "";

        if (typeHandle) {
            return fmt::format("fn_{}_{}_{}", typeHandle.toId(), typeHandle.toIndex(), scriptId);
        }

        // NOTE KI global scriplet
        return fmt::format("fn_global_{}", scriptId);
    }

    bool ScriptSystem::unregisterScriptEntry(const script::ScriptEntry& scriptEntry)
    {
        auto& lua = getLua();

        const auto& fnName = scriptEntry.m_signature;

        if (!lua[fnName].is<sol::function>()) return false;

        std::string undef = fmt::format(
            "{} = nil",
            fnName);

        lua.script(undef);
        KI_INFO_OUT(fmt::format("SCRIPT::UNREGISTER: function={}", scriptEntry.m_signature));
        return true;
    }

    void ScriptSystem::runGlobalScript(
        const Node* node,
        script::script_id scriptId)
    {
        std::lock_guard lock(m_lock);

        const auto& it = m_scriptEntries.find(node->m_typeHandle);

        if (it == m_scriptEntries.end()) return;

        if (const auto& fnIt = it->second.find(scriptId);
            fnIt != it->second.end())
        {
            const auto& scriptEntry = fnIt->second;
            const auto& fnName = scriptEntry.m_signature;

            invokeLuaFunction([this, &fnName]() {
                sol::protected_function fn(getLua()[fnName]);
                return fn();
                });
        }
    }

    void ScriptSystem::runNodeScript(
        const Node* node,
        script::script_id scriptId)
    {
        if (!node) return;

        std::lock_guard lock(m_lock);

        const auto& nodeHandle = node->toHandle();
        const auto& it = m_scriptEntries.find(node->m_typeHandle);

        if (it == m_scriptEntries.end()) return;

        if (const auto& fnIt = it->second.find(scriptId);
            fnIt != it->second.end())
        {
            const auto& scriptEntry = fnIt->second;
            const auto& fnName = scriptEntry.m_signature;

            KI_INFO_OUT(fmt::format("SCRIPT::RUN: function={} - {}", fnName, node->getName()));

            invokeLuaFunction([this, nodeHandle, &fnName]() {
                sol::optional<sol::table> nodeState = getLua()[TABLE_STATES][nodeHandle.toId()];
                if (nodeState) {
                    sol::table t = nodeState.value();
                    sol::protected_function fn(t[fnName]);
                    return fn(nodeState);
                }
                return sol::protected_function_result{ getLua(), 0, 0, 0, sol::call_status::runtime };
            });

            execScript("Updater:refresh()");
        }
    }

    sol::protected_function_result ScriptSystem::execScript(
        const std::string& script)
    {
        //std::lock_guard lock(m_lock);
        return invokeLuaScript(script);
    }

    sol::protected_function_result ScriptSystem::execRepl(
        const std::string& script)
    {
        //std::lock_guard lock(m_lock);

        auto result = invokeLuaScript(script);
        if (!result.valid()) {
            auto res2 = invokeLuaScript("return " + script);
            // NOTE KI retain error from original;
            // seeing error with "return" is confusing
            if (res2.valid()) {
                return res2;
            }
        }
        return result;
    }

    bool ScriptSystem::hasFunction(
        pool::NodeHandle nodeHandle,
        std::string_view name)
    {
        //std::lock_guard lock(m_lock);

        sol::optional<sol::table> nodeState = getLua()[TABLE_STATES][nodeHandle.toId()];
        if (!nodeState) return false;

        sol::optional<sol::function> fnPtr = nodeState.value()[name];
        return fnPtr != sol::nullopt;
    }

    void ScriptSystem::invokeNodeFunction(
        const Node* node,
        bool self,
        const sol::function& fn,
        const sol::table& args)
    {
        //std::lock_guard lock(m_lock);

        invokeLuaFunction([this, node, self, &fn, &args]() {
            sol::optional<sol::table> nodeState = getLua()[TABLE_STATES][node->getId()];
            if (nodeState) {
                return self ? fn(nodeState, args) : fn(args);
            }
            return sol::protected_function_result{ getLua(), 0, 0, 0, sol::call_status::runtime };
        });
    }

    void ScriptSystem::emitEvent(
        int listenerId,
        int type,
        const std::string& data)
    {
        //std::lock_guard lock(m_lock);

        invokeLuaFunction([this, &type, &data, &listenerId]() {
            sol::table events = getLua()["events"];
            sol::protected_function fn(events["emit_raw"]);
            return fn(events, type, data, listenerId);
            });
    }

    sol::protected_function_result ScriptSystem::invokeLuaFunction(
        const std::function<sol::protected_function_result()>& fn)
    {
        std::string error;
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
            error = ex.what();
        }
        catch (...) {
            error = "UNKNOWN_ERROR";
        }
        KI_CRITICAL(fmt::format("SCRIPT::RUNTIME: {}", error));

        return { getLua(), 0, 0, 0, sol::call_status::runtime };
    }

    // https://developercommunity.visualstudio.com/t/exception-block-is-optmized-away-causing-a-crash/253077
    sol::protected_function_result ScriptSystem::invokeLuaScript(
        const std::string& script)
    {
        std::string error;
        try {
            KI_INFO_OUT(util::appendLineNumbers(script));
            return getLua().safe_script(script);
        }
        catch (const std::exception& ex) {
            error = ex.what();
        }
        catch (...) {
            error = "UNKNOWN_ERROR";
        }
        KI_CRITICAL(fmt::format("SCRIPT::RUNTIME: {}", error));

        return { getLua(), 0, 0, 0, sol::call_status::runtime };
    }
}
