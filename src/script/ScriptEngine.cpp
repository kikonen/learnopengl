#include "ScriptEngine.h"

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
#include "script/NodeAPI.h"
#include "script/NodeCommandAPI.h"
#include "script/UtilAPI.h"
#include "script/ScriptFile.h"
#include "script/ScriptEntry.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "user_type/LuaUtil.h"
#include "user_type/LuaNode.h"
#include "user_type/LuaCommand.h"
#include "user_type/LuaGlm.h"
#include "user_type/LuaRayHit.h"
#include "user_type/LuaPhysics.h"

namespace
{
    const static std::string TABLE_CLASSES = "classes";
    const static std::string TABLE_STATES = "states";

    static script::ScriptEngine* s_engine{ nullptr };
}

namespace script
{
    void ScriptEngine::init() noexcept
    {
        s_engine = new ScriptEngine();
    }

    void ScriptEngine::release() noexcept
    {
        auto* s = s_engine;
        s_engine = nullptr;
        delete s;
    }

    ScriptEngine& ScriptEngine::get() noexcept
    {
        assert(s_engine);
        return *s_engine;
    }
}

namespace script
{
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

        auto& lua = getLua();

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

        lua[TABLE_CLASSES] = lua.create_table_with();
        lua[TABLE_STATES] = lua.create_table_with();
    }

    void ScriptEngine::update(const UpdateContext& ctx)
    {
        invokeLuaFunction([this, &ctx]() {
            auto updater = getLua()["Updater"];
            sol::protected_function fn(updater["update"]);
            return fn(updater, ctx.m_clock.elapsedSecs);
            });
    }

    void ScriptEngine::registerTypes()
    {
        // util - static utils
        auto& lua = getLua();

        LuaUtil::bind(lua);
        LuaGlm::bind(lua);
        LuaPhysics::bind(lua);
        LuaRayHit::bind(lua);
        LuaCommand::bind(lua);
        LuaNode::bind(lua);
    }

    script::script_id ScriptEngine::registerScript(
        script::ScriptFile scriptFile)
    {
        std::lock_guard lock(m_lock);

        scriptFile.m_id = script::ScriptFile::nextId();

        m_scripts.insert({ scriptFile.m_id, scriptFile });

        return scriptFile.m_id;
    }

    void ScriptEngine::bindTypeScript(
        pool::TypeHandle handle,
        script::script_id scriptId)
    {
        std::lock_guard lock(m_lock);

        {
            const auto& signature = getScriptSignature(handle, scriptId);
            if (signature.empty()) return;
        }

        auto it = m_scriptEntries.find(handle);
        if (it == m_scriptEntries.end()) {
            std::unordered_map<script::script_id, script::ScriptEntry> scriptMap;
            m_scriptEntries.insert({ handle, scriptMap });
            it = m_scriptEntries.find(handle);
        }

        {
            const auto& scriptEntry = createScriptEntry(handle, scriptId);
            if (scriptEntry.m_valid) {
                it->second.insert({ scriptId, scriptEntry });
            }
        }
    }

    void ScriptEngine::bindNodeScript(
        Node* node,
        script::script_id scriptId)
    {
        std::lock_guard lock(m_lock);

        const auto& handle = node->toHandle();
        if (handle.isNull()) return;

        if (!hasScriptEntry(node->m_typeHandle, scriptId)) return;

        {
            m_nodeApis.insert({ handle, std::make_unique<NodeAPI>(handle) });
            m_nodeCommandApis.insert({ handle, std::make_unique<NodeCommandAPI>(m_commandEngine, handle) });
        }

        createNodeState(node);
    }

    void ScriptEngine::createNodeState(
        Node* node)
    {
        const auto id = node->getId();
        const auto typeId = node->getType()->getId();
        std::string scriptlet = fmt::format(R"(
  local State = classes[{}]
  states[{}] = State:new()
  State = nil
  )", typeId, id);
        auto result = invokeLuaScript(scriptlet);
        if (!result.valid()) return;

        const auto handle = node->toHandle();
        auto* nodeApi = m_nodeApis.find(handle)->second.get();
        auto* cmdApi = m_nodeCommandApis.find(handle)->second.get();

        sol::table nodeState = getLua()[TABLE_STATES][handle.toId()];
        nodeState["id"] = node->getId();
        nodeState["node"] = std::ref(nodeApi);
        nodeState["cmd"] = std::ref(cmdApi);
    }

    bool ScriptEngine::hasScriptEntry(
        pool::TypeHandle handle,
        script::script_id scriptId)
    {
        const auto& typeIt = m_scriptEntries.find(handle);
        if (typeIt == m_scriptEntries.end()) return false;

        const auto& scriptMap = typeIt->second;
        const auto& scriptIt = scriptMap.find(scriptId);
        if (scriptIt == scriptMap.end()) return false;
        return scriptIt->second.m_valid;
    }

    std::vector<script::script_id> ScriptEngine::getScriptEntryIds(
        pool::TypeHandle handle)
    {
        std::lock_guard lock(m_lock);

        const auto& it = m_scriptEntries.find(handle);
        if (it == m_scriptEntries.end()) return {};

        std::vector<script::script_id> scripts;
        for (const auto& fnIt : it->second) {
            scripts.push_back(fnIt.first);
        }
        return scripts;
    }

    script::ScriptEntry ScriptEngine::createScriptEntry(
        pool::TypeHandle handle,
        script::script_id scriptId)
    {
        const auto typeId = handle.toId();

        // NOTE KI unique wrapperFn for node
        const std::string& fnName = getScriptSignature(handle, scriptId);
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

        if (handle) {
            std::string classScriptlet = fmt::format(R"(
classes[{}] = classes[{}] or Node:new({{ type_id={} }})
)", typeId, typeId, typeId);

            // NOTE KI pass context as closure to Node
            // - node, cmd, id
            scriptlet = fmt::format(R"(
{}
classes[{}].{} = function(self)
local State = getmetatable(self)
local cmd = self.cmd
local node = self.node
{}
end)", classScriptlet, typeId, fnName, scriptFile.m_source);
        }
        else {
            // NOTE KI global scriplet

            scriptlet = fmt::format(R"(
function {}()
{}
end)", fnName, scriptFile.m_source);
        }

        auto result = invokeLuaScript(scriptlet);
        if (!result.valid()) return { false };

        return { true, ScriptEntryType::function, fnName };
    }

    std::string ScriptEngine::getScriptSignature(
        pool::TypeHandle handle,
        script::script_id scriptId) const
    {
        const auto it = m_scripts.find(scriptId);
        if (it == m_scripts.end()) return "";

        if (handle) {
            return fmt::format("fn_{}_{}_{}", handle.toId(), handle.toIndex(), scriptId);
        }

        // NOTE KI global scriplet
        return fmt::format("fn_global_{}", scriptId);
    }

    bool ScriptEngine::unregisterScriptEntry(const script::ScriptEntry& scriptEntry)
    {
        auto& lua = getLua();

        const auto& fnName = scriptEntry.m_signature;

        if (!lua[fnName].is<sol::function>()) return false;

        std::string undef = fmt::format(R"(
{} = nil)", fnName);

        lua.script(undef);
        KI_INFO_OUT(fmt::format("SCRIPT::UNREGISTER: function={}", scriptEntry.m_signature));
        return true;
    }

    void ScriptEngine::runGlobalScript(
        script::script_id scriptId)
    {
        std::lock_guard lock(m_lock);

        const auto& it = m_scriptEntries.find(pool::TypeHandle::NULL_HANDLE);

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

    void ScriptEngine::runNodeScript(
        Node* node,
        script::script_id scriptId)
    {
        if (!node) return;

        std::lock_guard lock(m_lock);

        const auto& handle = node->toHandle();
        const auto& it = m_scriptEntries.find(node->m_typeHandle);

        if (it == m_scriptEntries.end()) return;

        if (const auto& fnIt = it->second.find(scriptId);
            fnIt != it->second.end())
        {
            const auto& scriptEntry = fnIt->second;
            const auto& fnName = scriptEntry.m_signature;

            KI_INFO_OUT(fmt::format("SCRIPT::RUN: function={} - {}", fnName, node->getName()));

            invokeLuaFunction([this, handle, &fnName]() {
                sol::table nodeState = getLua()[TABLE_STATES][handle.toId()];
                sol::protected_function fn(nodeState[fnName]);
                return fn(nodeState);
            });
        }
    }

    sol::protected_function_result ScriptEngine::execScript(
        const std::string& script)
    {
        std::lock_guard lock(m_lock);
        return invokeLuaScript(script);
    }

    sol::protected_function_result ScriptEngine::execRepl(
        const std::string& script)
    {
        std::lock_guard lock(m_lock);

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

    bool ScriptEngine::hasFunction(
        pool::NodeHandle handle,
        std::string_view name)
    {
        std::lock_guard lock(m_lock);

        sol::table nodeState = getLua()[TABLE_STATES][handle.toId()];

        sol::optional<sol::function> fnPtr = nodeState[name];
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
            sol::table nodeState = getLua()[TABLE_STATES][node->getId()];
            return self ? fn(nodeState, args) : fn(args);
            });
    }

    void ScriptEngine::emitEvent(
        int listenerId,
        int type,
        const std::string& data)
    {
        std::lock_guard lock(m_lock);

        invokeLuaFunction([this, &type, &data, &listenerId]() {
            sol::table events = getLua()["events"];
            sol::protected_function fn(events["emit_raw"]);
            return fn(events, type, data, listenerId);
            });
    }

    sol::protected_function_result ScriptEngine::invokeLuaFunction(
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
    sol::protected_function_result ScriptEngine::invokeLuaScript(
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
