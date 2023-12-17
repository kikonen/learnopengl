#pragma once

#include <unordered_map>
#include <memory>
#include <map>
#include <mutex>

#include <sol/sol.hpp>

#include "asset/Assets.h"

#include "ki/size.h"

#include "size.h"
#include "Script.h"

class Node;

namespace script
{
    class CommandEngine;
    class CommandAPI;

    class ScriptEngine final
    {
    public:
        ScriptEngine(const Assets& assets);
        ~ScriptEngine() = default;

        void prepare(
            CommandEngine* commandEngine);

        script::script_id registerScript(std::string_view source);

        void bindNodeScript(
            ki::object_id nodeId,
            script::script_id scriptId);

        std::vector<script::script_id> getNodeScripts(
            ki::object_id nodeId);

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
            ki::object_id nodeId,
            script::script_id scriptId);

        void registerTypes();

    private:
        const Assets& m_assets;

        CommandEngine* m_commandEngine{ nullptr };

        sol::state m_lua;
        sol::table m_luaNodes;

        std::unordered_map<ki::object_id, std::unique_ptr<CommandAPI>> m_apis;

        std::unordered_map<ki::object_id, std::unordered_map<script::script_id, std::string>> m_nodeFunctions;
        std::unordered_map<ki::object_id, std::vector<script::script_id>> m_nodeScripts;

        std::unordered_map<script::script_id, Script> m_scripts;

        std::mutex m_lock{};
    };
}
