#include "ScriptEngine.h"

#include <map>

#include <fmt/format.h>

#include "util/Util.h"

#include "command/CommandEngine.h"
#include "command/CommandAPI.h"

#include "model/Node.h"

#include "registry/MeshType.h"


namespace {
    const std::string INIT_FN{ "init" };
    const std::string RUN_FN{ "run" };
    const std::string NONE_FN{ "<missing>" };

    const std::string& scriptIdToString(NodeScriptId scriptId) {
        switch (scriptId) {
        case NodeScriptId::init:
            return INIT_FN;
        case NodeScriptId::run:
            return RUN_FN;
        }
        return NONE_FN;
    }
}

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

        ut["resume"] = sol::yielding(& CommandAPI::lua_resume);
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

void ScriptEngine::runScript(
    Node* node,
    const NodeScriptId scriptId)
{
    const auto& nodeIt = m_nodeScripts.find(node->m_objectID);
    if (nodeIt == m_nodeScripts.end()) return;

    const auto& fnIt = nodeIt->second.find(scriptId);
    if (fnIt == nodeIt->second.end()) return;

    const auto& nodeFnName = fnIt->second;

    sol::function fn = m_lua[nodeFnName];

    const auto& it = m_apis.find(node->m_objectID);
    auto* api = it->second.get();

    fn(std::ref(node), std::ref(api), node->m_objectID);
}

void ScriptEngine::registerNode(
    Node* node)
{
    m_luaNodes[node->m_objectID] = m_lua.create_table_with();
}

void ScriptEngine::registerScript(
    Node* node,
    const NodeScriptId scriptId,
    std::string_view script)
{
    if (script.empty()) return;

    const auto& scriptName = scriptIdToString(scriptId);

    // NOTE KI unique wrapperFn for node
    const std::string nodeFnName = fmt::format("fn_{}_{}", scriptName, node->m_objectID);

    // NOTE KI pass context as closure to Node
    // - node, cmd, id
    const auto scriptlet = fmt::format(R"(
function {}(node, cmd, id)
{}
end)", nodeFnName, script);

    m_lua.script(scriptlet);

    sol::table luaNode = m_luaNodes[node->m_objectID];
    luaNode[scriptName] = m_lua[nodeFnName];

    m_nodeScripts[node->m_objectID][scriptId] = nodeFnName;

    m_apis.insert({ node->m_objectID, std::make_unique<CommandAPI>(this, m_commandEngine, node->m_objectID) });
}

bool ScriptEngine::hasFunction(
    Node* node,
    std::string_view name)
{
    sol::table luaNode = m_luaNodes[node->m_objectID];

    sol::optional<sol::function> fnPtr = luaNode[name];
    return fnPtr != sol::nullopt;
}

void ScriptEngine::invokeFunction(
    Node* node,
    std::string_view name)
{
    //KI_INFO_OUT(fmt::format("CALL LUA: name={}, id={}, fn={}", node->m_type->m_name, node->m_objectID, name));
    sol::table luaNode = m_luaNodes[node->m_objectID];

    sol::optional<sol::function> fnPtr = luaNode[name];
    if (fnPtr != sol::nullopt) {
        fnPtr.value()(std::ref(node), node->m_objectID);
    }
}
