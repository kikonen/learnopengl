#include "ScriptEngine.h"

#include <fmt/format.h>

#include "model/Node.h"

#include "command/CommandEngine.h"
#include "command/CommandAPI.h"

namespace {
    std::string scriptIdToString(NodeScriptId scriptId) {
        switch (scriptId) {
        case NodeScriptId::init:
            return "init";
        case NodeScriptId::run:
            return "init";
        }
        return "WTF";
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
    Node& node,
    const NodeScriptId scriptId)
{
    const auto& nodeIt = m_nodeScripts.find(node.m_objectID);
    if (nodeIt == m_nodeScripts.end()) return;
    const auto& fnIt = nodeIt->second.find(scriptId);
    if (fnIt == nodeIt->second.end()) return;

    const auto& nodeFnName = fnIt->second;
    sol::function fn = m_lua[nodeFnName];
    fn(node, node.m_objectID);
}

void ScriptEngine::registerScript(
    Node& node,
    const NodeScriptId scriptId,
    const std::string& script)
{
    if (script.empty()) return;

    // NOTE KI unique wrapperFn for node
    const std::string nodeFnName = fmt::format("fn_{}_{}", scriptIdToString(scriptId), node.m_objectID);
    const auto scriptlet = fmt::format(R"(
function {}(node, id)
{}
end)", nodeFnName, script);

    m_lua.script(scriptlet);

    m_nodeScripts[node.m_objectID][scriptId] = nodeFnName;
}
