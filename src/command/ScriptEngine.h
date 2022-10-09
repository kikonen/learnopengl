#pragma once

#include <map>

#include <sol/sol.hpp>

#include "model/Node.h"

class ScriptEngine
{
public:
    ScriptEngine();

    void runScript(
        Node& node,
        const NodeScriptId scriptId);

    void registerScript(
        Node& node,
        const NodeScriptId scriptId,
        const std::string& script);

private:
    void registerTypes();


private:
    sol::state m_lua;

    std::map<int, std::map<NodeScriptId, std::string>> nodeScripts;
};
