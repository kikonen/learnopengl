#pragma once

#include <unordered_map>
#include <memory>
#include <map>
#include <mutex>
#include <functional>

#include <sol/sol.hpp>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "size.h"
#include "Script.h"

struct PrepareContext;

class Node;

namespace script
{
    class CommandEngine;
    class NodeCommandAPI;
    //class UtilAPI;

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
            bool self,
            const sol::function& fn,
            const sol::table& args);

        void emitEvent(
            int listenerId,
            int type,
            const std::string& data);

        sol::state* getLua() { return m_state.get(); }

    private:
        inline sol::state& getState() { return *m_state; }

        sol::protected_function_result invokeLuaFunction(
            const std::function<sol::protected_function_result()>& fn);

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

        std::unique_ptr<sol::state> m_state;

        std::unordered_map<pool::NodeHandle, std::unique_ptr<NodeCommandAPI>> m_nodeCommandApis;

        std::unordered_map<pool::NodeHandle, std::unordered_map<script::script_id, std::string>> m_nodeFunctions;

        std::unordered_map<script::script_id, Script> m_scripts;

        std::mutex m_lock{};
    };
}
