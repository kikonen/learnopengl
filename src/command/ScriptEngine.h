#pragma once

#include <unordered_map>
#include <memory>

#include <sol/sol.hpp>

#include "asset/Assets.h"


class CommandEngine;
class CommandAPI;
class Node;
enum class NodeScriptId;

class ScriptEngine final
{
public:
    ScriptEngine(const Assets& assets);
    ~ScriptEngine()= default;

    void prepare(
        CommandEngine* commandEngine);

    void registerNode(
        Node* node);

    void runScript(
        Node* node,
        const NodeScriptId scriptId);

    void registerScript(
        Node* node,
        const NodeScriptId scriptId,
        const std::string& script);

    bool hasFunction(
        Node* node,
        const std::string& name);

    void invokeFunction(
        Node* node,
        const std::string& name);

private:
    void registerTypes();


private:
    const Assets& m_assets;

    sol::state m_lua;
    sol::thread m_runner;

    std::unique_ptr<CommandAPI> m_commandAPI;

    std::unordered_map<int, std::unordered_map<NodeScriptId, std::string>> m_nodeScripts;
};
