#include "ScriptEngine.h"

#include <map>

#include <fmt/format.h>

#include "util/Util.h"

#include "script/CommandEngine.h"
#include "script/CommandAPI.h"

#include "model/Node.h"

#include "registry/MeshType.h"

namespace {
}

namespace script
{

    ScriptEngine::ScriptEngine(const Assets& assets)
        : m_assets(assets)
    {
    }

    void ScriptEngine::prepare(
        CommandEngine* commandEngine)
    {
        m_commandEngine = commandEngine;

        m_lua.open_libraries(sol::lib::base);
        m_lua.open_libraries(sol::lib::math);
        m_lua.open_libraries(sol::lib::os);
        m_lua.open_libraries(sol::lib::coroutine);
        m_lua.open_libraries(sol::lib::string);

        registerTypes();

        m_lua["nodes"] = m_lua.create_table_with();
        m_luaNodes = m_lua["nodes"];
    }

    void ScriptEngine::registerTypes()
    {
        // CommandAPI
        {
            m_lua.new_usertype<CommandAPI>("CommandAPI");

            const auto& ut = m_lua["CommandAPI"];

            ut["cancel"] = &CommandAPI::lua_cancel;
            ut["wait"] = &CommandAPI::lua_wait;
            ut["sync"] = &CommandAPI::lua_sync;

            ut["move"] = &CommandAPI::lua_move;
            ut["moveSpline"] = &CommandAPI::lua_moveSpline;
            ut["rotate"] = &CommandAPI::lua_rotate;
            ut["scale"] = &CommandAPI::lua_scale;

            ut["audioPlay"] = &CommandAPI::lua_audioPlay;
            ut["audioPause"] = &CommandAPI::lua_audioPause;
            ut["audioStop"] = &CommandAPI::lua_audioStop;

            ut["resume"] = sol::yielding(&CommandAPI::lua_resume);
            ut["start"] = &CommandAPI::lua_start;
        }

        // Node
        {
            m_lua.new_usertype<Node>("Node");

            const auto& ut = m_lua["Node"];

            ut["getId"] = &Node::lua_getId;
            ut["getName"] = &Node::lua_getName;

            ut["getCloneIndex"] = &Node::lua_getCloneIndex;

            ut["getPos"] = &Node::lua_getPos;
        }
    }

    script::script_id ScriptEngine::registerScript(std::string_view source)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        Script script{ source };
        m_scripts.insert({ script.m_id, script });

        return script.m_id;
    }

    void ScriptEngine::registerNodeScript(
        ki::object_id nodeId,
        script::script_id scriptId)
    {
        auto fnName = createNodeFunction(nodeId, scriptId);

        if (fnName.empty()) return;

        std::unordered_map<script::script_id, std::string> fnMap{ { scriptId, fnName } };
        m_nodeFunctions.insert({ nodeId, fnMap });

        m_apis.insert({ nodeId, std::make_unique<CommandAPI>(this, m_commandEngine, nodeId) });
    }

    std::string ScriptEngine::createNodeFunction(
        ki::object_id nodeId,
        script::script_id scriptId)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        const auto it = m_scripts.find(scriptId);
        if (it == m_scripts.end()) return "";

        const auto& script = it->second;

        // NOTE KI unique wrapperFn for node
        const std::string nodeFnName = fmt::format("fn_{}_{}", nodeId, scriptId);

        // NOTE KI pass context as closure to Node
        // - node, cmd, id
        const auto scriptlet = fmt::format(R"(
function {}(node, cmd, id)
local luaNode = nodes[id]
{}
end)", nodeFnName, script.m_source);

        m_lua.script(scriptlet);

        return nodeFnName;
    }

    std::vector<std::string> ScriptEngine::runScripts(
        Node* node)
    {
        std::vector<std::string> functions;

        const auto nodeId = node->m_id;
        const auto& it = m_nodeFunctions.find(nodeId);

        if (it == m_nodeFunctions.end()) return {};

        for (const auto& fnIt : it->second)
        {
            const auto& fnName = fnIt.second;

            sol::function fn = m_lua[fnName];
            auto* api = m_apis.find(nodeId)->second.get();
            fn(std::ref(node), std::ref(api), nodeId);

            functions.push_back(fnName);
        }

        return functions;
    }

    void ScriptEngine::registerNode(
        Node* node)
    {
        m_luaNodes[node->m_id] = m_lua.create_table_with();
    }

    bool ScriptEngine::hasFunction(
        Node* node,
        std::string_view name)
    {
        sol::table luaNode = m_luaNodes[node->m_id];

        sol::optional<sol::function> fnPtr = luaNode[name];
        return fnPtr != sol::nullopt;
    }

    void ScriptEngine::invokeFunction(
        Node* node,
        std::string_view name)
    {
        const auto nodeId = node->m_id;

        //KI_INFO_OUT(fmt::format("CALL LUA: name={}, id={}, fn={}", node->m_type->m_name, node->m_id, name));
        sol::table luaNode = m_luaNodes[nodeId];

        sol::optional<sol::function> fnPtr = luaNode[name];
        if (fnPtr != sol::nullopt) {
            auto* api = m_apis.find(nodeId)->second.get();
            auto& fn = fnPtr.value();
            fn(std::ref(node), std::ref(api), nodeId);
        }
    }
}
