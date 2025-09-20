#pragma once

#include <unordered_map>
#include <memory>
#include <map>
#include <mutex>
#include <functional>

#include "script/lua_binding.h"

#include "ki/size.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "size.h"

struct PrepareContext;
struct UpdateContext;

class Node;

namespace script
{
    namespace api {
        class NodeAPI;
        class NodeCommandAPI;
        class SceneAPI;
    }

    class CommandEngine;

    struct ScriptFile;
    struct ScriptEntry;

    class ScriptSystem final
    {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static ScriptSystem& get() noexcept;

        ScriptSystem();
        ScriptSystem& operator=(const ScriptSystem&) = delete;

        ~ScriptSystem();

        void clear();

        void shutdown();

        void prepare(
            const PrepareContext& ctx,
            CommandEngine* commandEngine);

        void update(const UpdateContext& ctx);

        script::script_id registerScript(
            script::ScriptFile scriptFile);

        void bindTypeScript(
            bool global,
            pool::TypeHandle typeHandle,
            script::script_id scriptId);

        void bindNode(
            const Node* node);

        void unbindNode(
            const Node* node);

        std::string getScriptSignature(
            pool::TypeHandle typeHandle,
            script::script_id scriptId) const;

        bool hasScriptEntry(
            pool::TypeHandle typeHandle,
            script::script_id scriptId);

        std::vector<script::script_id> getScriptEntryIds(
            pool::TypeHandle typeHandle);

        void runGlobalScript(
            const Node* node,
            script::script_id scriptId);

        void runNodeScript(
            const Node* node,
            script::script_id scriptId);

        bool hasFunction(
            pool::NodeHandle nodeHandle,
            std::string_view name);

        void invokeNodeFunction(
            const Node* node,
            bool self,
            const sol::function& fn,
            const sol::table& args);

        void emitEvent(
            int listenerId,
            int type,
            const std::string& data);

        sol::protected_function_result execScript(
            const std::string& script);

        sol::protected_function_result execRepl(
            const std::string& script);

        inline sol::state& getLua() noexcept{ return m_lua; }

    private:
        void createNodeState(
            const Node* node);

        void deleteNodeState(
            const Node* node);

        sol::protected_function_result invokeLuaFunction(
            const std::function<sol::protected_function_result()>& fn);

        sol::protected_function_result invokeLuaScript(
            const std::string& script);

        script::ScriptEntry createScriptEntry(
            bool global,
            pool::TypeHandle typeHandle,
            script::script_id scriptId);

        // @return true if unregister was done
        bool unregisterScriptEntry(const script::ScriptEntry& scriptEntry);

        void registerTypes();

    private:
        CommandEngine* m_commandEngine{ nullptr };

        sol::state m_lua;

        std::unique_ptr<api::SceneAPI> m_sceneApi;

        std::unique_ptr<api::NodeCommandAPI> m_nodeCommandApi;

        std::unordered_map<pool::NodeHandle, std::unique_ptr<api::NodeAPI>> m_nodeApis;
        std::unordered_map<pool::NodeHandle, std::unique_ptr<api::NodeCommandAPI>> m_nodeCommandApis;

        std::unordered_map<pool::TypeHandle, std::unordered_map<script::script_id, ScriptEntry>> m_scriptEntries;

        std::unordered_map<script::script_id, script::ScriptFile> m_scripts;

        std::mutex m_lock{};
    };
}
