#include "ScriptEngine.h"

#include <fmt/format.h>

#include "model/Node.h"


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
    const std::string& script,
    const std::string& functionName)
{
    if (script.empty()) return;

    auto p = node.getPos();
    std::cout << fmt::format("BEFORE: SOL: {} - pos=({}, {}, {})\n", node.str(), p.x, p.y, p.z);

    m_lua.script(script);
    sol::function fn = m_lua[functionName];
    fn(node);

    p = node.getPos();
    fmt::format("BEFORE: SOL: {} - pos=({}, {}, {})\n", node.str(), p.x, p.y, p.z);

}
