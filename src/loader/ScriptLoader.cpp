#include "ScriptLoader.h"

#include <filesystem>

#include "asset/Assets.h"

#include "util/util.h"

#include "model/Node.h"

#include "script/ScriptEngine.h"
#include "event/Dispatcher.h"
#include "registry/Registry.h"

#include "loader/document.h"
#include "loader_util.h"

namespace {
    const std::string LUA_EXT{ ".lua" };
}

namespace loader {
    ScriptLoader::ScriptLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void ScriptLoader::loadScriptEngine(
        const loader::DocNode& node,
        ScriptEngineData& data) const
    {
        data.enabled = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "xxenabled" || k == "xenabled") {
                // NOTE compat with old "disable" logic
                data.enabled = false;
            }
            else if (k == "scripts") {
                loadScripts(v, data.scripts, false);
            }
            else if (k == "script_files") {
                loadScripts(v, data.scripts, true);
            }
            else {
                reportUnknown("script_engine_entry", k, v);
            }
        }
    }

    void ScriptLoader::loadScripts(
        const loader::DocNode& node,
        std::vector<ScriptData>& scripts,
        bool forceFile) const
    {
        for (const auto& entry : node.getNodes()) {
            ScriptData& data = scripts.emplace_back();
            loadScript(entry, data, forceFile);
        }
    }

    void ScriptLoader::loadScript(
        const loader::DocNode& node,
        ScriptData& data,
        bool forceFile) const
    {
        data.enabled = true;

        if (node.isScalar()) {
            const auto& str = readString(node);
            const auto& scriptPath = resolveScriptPath(str);

            if (forceFile || fileExists(scriptPath)) {
                data.script = readFile(scriptPath);
            }
            else {
                data.script = str;
            }
            return;
        }

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "xxenabled" || k == "xenabled") {
                // NOTE compat with old "disable" logic
                data.enabled = false;
            }
            else if (k == "path") {
                data.path = resolveScriptPath(readString(v));
            }
            else if (k == "script") {
                data.script = readString(v);
            }
            else {
                reportUnknown("script_entry", k, v);
            }
        }

        if (data.path.empty() && data.script.empty()) {
            data.enabled = false;
        }
    }

    void ScriptLoader::createScriptEngine(
        const ScriptEngineData& data)
    {
        if (!data.enabled) return;
        const auto& registeredIds = createScripts(pool::NodeHandle::NULL_HANDLE, data.scripts);
        bindNodeScripts(pool::NodeHandle::NULL_HANDLE, registeredIds);
        runGlobalScripts(registeredIds);
    }

    std::vector<script::script_id> ScriptLoader::createScripts(
        pool::NodeHandle handle,
        const std::vector<ScriptData>& scripts) const
    {
        std::vector<script::script_id> registeredIds;

        for (auto& data : scripts) {
            for (auto scriptId : createScript(handle, data)) {
                registeredIds.push_back(scriptId);
            }
        }

        return registeredIds;
    }

    std::vector<script::script_id> ScriptLoader::createScript(
        pool::NodeHandle handle,
        const ScriptData& data) const
    {
        if (!data.enabled) return {};

        std::vector<script::script_id> registeredIds;

        std::vector<std::string> scripts;

        if (!data.path.empty()) {
            scripts.push_back(readFile(data.path));
        }
        if (!data.script.empty()) {
            scripts.push_back(data.script);
        }

        for (const auto& script : scripts) {
            if (script.empty()) continue;
            auto scriptId = script::ScriptEngine::get().registerScript(script);
            registeredIds.push_back(scriptId);
        }

        return registeredIds;
    }

    void ScriptLoader::bindNodeScripts(
        pool::NodeHandle handle,
        const std::vector<script::script_id>& scriptIds) const
    {
        const auto& assets = Assets::get();
        if (!assets.useScript) return;

        for (auto scriptId : scriptIds)
        {
            event::Event evt { event::Type::script_bind };
            auto& body = evt.body.script = {
                .target = handle.toId(),
                .id = scriptId,
            };
            m_dispatcher->send(evt);
        }
    }

    void ScriptLoader::runGlobalScripts(
        const std::vector<script::script_id>& scriptIds) const
    {
        for (auto scriptId : scriptIds) {
            event::Event evt { event::Type::script_run };
            auto& body = evt.body.script = {
                .target = 0,
                .id = scriptId,
            };
            m_dispatcher->send(evt);
        }
    }

    std::string ScriptLoader::resolveScriptPath(const std::string& str) const
    {
        std::filesystem::path scriptPath{ str };

        if (scriptPath.extension() == LUA_EXT) {
            return str;
        }

        if (!fileExists(scriptPath.string()))
        {
            scriptPath += LUA_EXT;
        }
        return scriptPath.string();
    }
}
