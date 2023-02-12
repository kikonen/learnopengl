#include "ScriptEngine.h"

#include <iostream>

#include <fmt/format.h>

#include "util/Util.h"

#include "command/CommandEngine.h"
#include "command/CommandAPI.h"

#include "model/Node.h"

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
    CommandEngine& commandEngine)
{
    m_runner = sol::thread::create(m_lua);
    m_commandAPI = std::make_unique<CommandAPI>(commandEngine, m_runner);

    m_lua.open_libraries(sol::lib::base);
    m_lua.open_libraries(sol::lib::math);
    m_lua.open_libraries(sol::lib::os);
    m_lua.open_libraries(sol::lib::coroutine);
    m_lua.open_libraries(sol::lib::string);

    registerTypes();

    m_lua.set("cmd", m_commandAPI.get());
    m_lua["nodes"] = m_lua.create_table_with();
}

void ScriptEngine::registerTypes()
{
    // CommandAPI
    {
        m_lua.new_usertype<CommandAPI>("CommandAPI");

        const auto& ut = m_lua["CommandAPI"];

        ut["cancel"] = &CommandAPI::lua_cancel;
        ut["wait"] = &CommandAPI::lua_wait;

        ut["move"] = &CommandAPI::lua_move;
        ut["moveSpline"] = &CommandAPI::lua_moveSpline;
        ut["rotate"] = &CommandAPI::lua_rotate;
        ut["scale"] = &CommandAPI::lua_scale;

        ut["resume"] = &CommandAPI::lua_resume;
        ut["start"] = &CommandAPI::lua_start;
    }

    // Node
    {
        m_lua.new_usertype<Node>("Node");

        const auto& ut = m_lua["Node"];
        ut["getId"] = &Node::lua_getId;
        ut["getPos"] = &Node::lua_getPos;
        ut["setPos"] = &Node::lua_setPos;
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
    fn(std::ref(node), node->m_objectID);
}

void ScriptEngine::registerNode(
    Node* node)
{
    sol::table nodes = m_lua["nodes"];
    nodes[node->m_objectID] = m_lua.create_table_with();
}

void ScriptEngine::registerScript(
    Node* node,
    const NodeScriptId scriptId,
    const std::string& script)
{
    if (script.empty()) return;

    const auto& scriptName = scriptIdToString(scriptId);

    // NOTE KI unique wrapperFn for node
    const std::string nodeFnName = fmt::format("fn_{}_{}", scriptName, node->m_objectID);
    const auto scriptlet = fmt::format(R"(
function {}(node, id)
{}
end)", nodeFnName, script);

    m_lua.script(scriptlet);

    sol::table luaNode = m_lua["nodes"][node->m_objectID];
    luaNode[scriptName] = m_lua[nodeFnName];

    sol::function fn = luaNode[scriptName];

    m_nodeScripts[node->m_objectID][scriptId] = nodeFnName;
}

void ScriptEngine::invokeFunction(
    Node* node,
    const std::string& callbackFn)
{
    KI_INFO_OUT(fmt::format("CALL LUA: name={}, id={}, fn={}", node->m_type->m_name, node->m_objectID, callbackFn));
    sol::table luaNode = m_lua["nodes"][node->m_objectID];
    sol::function fn = luaNode[callbackFn];
    fn(std::ref(node), node->m_objectID);
}
