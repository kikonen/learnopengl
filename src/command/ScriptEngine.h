#pragma once

#include <map>

#include <sol/sol.hpp>

#include "asset/Assets.h"

#include "model/Node.h"

class CommandEngine;

class ScriptEngine
{
public:
    ScriptEngine();

    void prepare(
        const Assets& assets,
        CommandEngine& commandEngine);

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
