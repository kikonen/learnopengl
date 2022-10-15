#pragma once

#include <map>
#include <memory>

#include <sol/sol.hpp>

#include "asset/Assets.h"

#include "model/Node.h"

#include "command/CommandAPI.h"

class CommandEngine;

class ScriptEngine final
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
    sol::thread m_runner;

    std::unique_ptr<CommandAPI> m_commandAPI;

    std::map<int, std::map<NodeScriptId, std::string>> m_nodeScripts;
};
