#pragma once

#include <unordered_map>
#include <memory>
#include <map>
#include <mutex>

#include <sol/sol.hpp>

#include "ki/size.h"

#include "size.h"
#include "Script.h"

struct PrepareContext;

class Node;

namespace script
{
    class CommandEngine;
    class CommandAPI;

    class ScriptEngine final
    {
    public:
        ScriptEngine();
        ~ScriptEngine() = default;

        void prepare(
            const PrepareContext& ctx,
            CommandEngine* commandEngine);

        script::script_id registerScript(std::string_view source);

        void bindNodeScript(
            ki::node_id nodeId,
            script::script_id scriptId);

        std::vector<script::script_id> getNodeScripts(
            ki::node_id nodeId);

        void runGlobalScript(
            script::script_id scriptId);

        void runNodeScript(
            Node* node,
            script::script_id scriptId);

        bool hasFunction(
            Node* node,
            std::string_view name);

        void invokeFunction(
            Node* node,
            std::string_view name);

        sol::state& getLua() { return m_lua; }

    private:
        // @return fnName
        std::string createNodeFunction(
            ki::node_id nodeId,
            script::script_id scriptId);

        void registerTypes();

    private:
        CommandEngine* m_commandEngine{ nullptr };

        sol::state m_lua;
        sol::table m_luaNodes;

        std::unordered_map<ki::node_id, std::unique_ptr<CommandAPI>> m_apis;

        std::unordered_map<ki::node_id, std::unordered_map<script::script_id, std::string>> m_nodeFunctions;
        std::unordered_map<ki::node_id, std::vector<script::script_id>> m_nodeScripts;

        std::unordered_map<script::script_id, Script> m_scripts;

        std::mutex m_lock{};
    };
}
