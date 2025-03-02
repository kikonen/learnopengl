#pragma once

#include <unordered_map>
#include <memory>
#include <map>
#include <mutex>

#include <sol/sol.hpp>

#include "ki/size.h"

#include "size.h"
#include "Script.h"

#include "pool/NodeHandle.h"

struct PrepareContext;

class Node;

namespace script
{
    class CommandEngine;
    class NodeCommandAPI;
    class UtilAPI;

    class ScriptEngine final
    {
    public:
        static ScriptEngine& get() noexcept;

        ScriptEngine();
        ScriptEngine& operator=(const ScriptEngine&) = delete;

        ~ScriptEngine();

        void clear();

        void shutdown();

        void prepare(
            const PrepareContext& ctx,
            CommandEngine* commandEngine);

        script::script_id registerScript(std::string_view source);

        void bindNodeScript(
            pool::NodeHandle handle,
            script::script_id scriptId);

        std::vector<script::script_id> getNodeScripts(
            pool::NodeHandle handle);

        void runGlobalScript(
            script::script_id scriptId);

        void runNodeScript(
            Node* node,
            script::script_id scriptId);

        bool hasFunction(
            pool::NodeHandle handle,
            std::string_view name);

        void invokeNodeFunction(
            Node* node,
            std::string_view fnName);

        void emitEvent(
            int listenerId,
            const std::string& type,
            const std::string& data);

        sol::state& getLua() { return m_lua; }

    private:
        const sol::protected_function_result& invokeLuaFunction(
            const std::function<const sol::protected_function_result&()>& fn);

        bool invokeLuaScript(
            const std::string& script);

        // @return fnName
        std::string createNodeFunction(
            pool::NodeHandle handle,
            script::script_id scriptId);

        // @return true if unregister was done
        bool unregisterFunction(std::string fnName);

        void registerTypes();

    private:
        CommandEngine* m_commandEngine{ nullptr };

        sol::state m_lua;
        sol::table m_luaNodes;

        std::unique_ptr<UtilAPI> m_utilApi;
        std::unordered_map<pool::NodeHandle, std::unique_ptr<NodeCommandAPI>> m_nodeCommandApis;

        std::unordered_map<pool::NodeHandle, std::unordered_map<script::script_id, std::string>> m_nodeFunctions;
        //std::unordered_map<pool::NodeHandle, std::vector<script::script_id>> m_nodeScripts;

        std::unordered_map<script::script_id, Script> m_scripts;

        std::mutex m_lock{};
    };
}
