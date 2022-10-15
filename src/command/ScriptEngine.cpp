#include "ScriptEngine.h"

#include <fmt/format.h>

#include "model/Node.h"

#include "command/CommandEngine.h"

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

ScriptEngine::ScriptEngine()
{
}

void ScriptEngine::prepare(
    const Assets& assets,
    CommandEngine& commandEngine)
{
    m_lua.open_libraries(sol::lib::base);
    m_lua.open_libraries(sol::lib::math);
    m_lua.open_libraries(sol::lib::os);

    registerTypes();

    m_lua.set("cmd", &commandEngine);
}

void ScriptEngine::registerTypes()
{
    // CommandEngine
    {
        m_lua.new_usertype<CommandEngine>("CommandEngine");

        const auto& ut = m_lua["CommandEngine"];

        ut["cancel"] = &CommandEngine::lua_cancel;
        ut["moveTo"] = &CommandEngine::lua_moveTo;
        ut["moveSplineTo"] = &CommandEngine::lua_moveSplineTo;
        ut["rotateTo"] = &CommandEngine::lua_rotateTo;
        ut["scaleTo"] = &CommandEngine::lua_scaleTo;
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
    const auto& nodeIt = nodeScripts.find(node.m_objectID);
    if (nodeIt == nodeScripts.end()) return;
    const auto& fnIt = nodeIt->second.find(scriptId);
    if (fnIt == nodeIt->second.end()) return;

    const auto& nodeFnName = fnIt->second;
    sol::function fn = m_lua[nodeFnName];
    fn(node);
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
function {}(node)
{}
end)", nodeFnName, script);

    m_lua.script(scriptlet);

    nodeScripts[node.m_objectID][scriptId] = nodeFnName;
}
