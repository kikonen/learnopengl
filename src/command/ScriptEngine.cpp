#include "ScriptEngine.h"

#include <fmt/format.h>

#include "model/Node.h"

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
    m_lua.open_libraries(sol::lib::base);
    registerTypes();
}

void ScriptEngine::registerTypes()
{
    m_lua.new_usertype<Node>("Node");

    const auto& ut = m_lua["Node"];
    ut["getPos"] = &Node::lua_getPos;
    ut["setPos"] = &Node::lua_setPos;
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
