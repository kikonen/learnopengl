#pragma once

#include <sol/sol.hpp>

class Node;

class ScriptEngine
{
public:
    ScriptEngine();

    void runScript(
        Node& node,
        const std::string& script,
        const std::string& functionName);

private:
    void registerTypes();

private:
    sol::state m_lua;
};
