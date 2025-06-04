#include "ScriptLoader.h"

#include <filesystem>
#include <regex>

#include "asset/Assets.h"

#include "util/util.h"

#include "model/Node.h"

#include "script/ScriptSystem.h"
#include "script/ScriptFile.h"
#include "event/Dispatcher.h"
#include "registry/Registry.h"

#include "loader/document.h"
#include "loader_util.h"

namespace {
    const std::string LUA_EXT{ ".lua" };
}

namespace loader {
    ScriptLoader::ScriptLoader(
        std::shared_ptr<Context> ctx)
        : BaseLoader(ctx)
    {
    }

    void ScriptLoader::loadScriptSystem(
        const loader::DocNode& node,
        ScriptSystemData& data) const
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

        for (auto& script : data.scripts) {
            if (script.type == script::ScriptType::class_file) {
                script.type = script::ScriptType::module_file;
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
                data.path = scriptPath;
            }
            else {
                data.script = str;
            }
            return;
        }

        bool hasType = false;

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
            else if (k == "type") {
                hasType = true;
                std::string type = readString(v);
                if (type == "plain") {
                    data.type = script::ScriptType::plain;
                }
                else if (type == "module") {
                    data.type = script::ScriptType::module_file;
                }
                else if (type == "class") {
                    data.type = script::ScriptType::class_file;
                }
                else {
                    reportUnknown("node_type", k, v);
                }
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

        if (!data.path.empty() && !hasType) {
            data.type = script::ScriptType::plain;

            if (std::regex_match(data.path, std::regex(".*class.*"))) {
                data.type = script::ScriptType::class_file;
            }
            else if (std::regex_match(data.path, std::regex(".*module.*"))) {
                data.type = script::ScriptType::module_file;
            }
        }

        if (data.path.empty() && data.script.empty()) {
            data.enabled = false;
        }
    }

    void ScriptLoader::createScriptSystem(
        const ScriptSystemData& data)
    {
        if (!data.enabled) return;
        const auto& registeredIds = createScripts(data.scripts);
        bindTypeScripts(pool::TypeHandle::NULL_HANDLE, registeredIds);
        runGlobalScripts(registeredIds);
    }

    std::vector<script::script_id> ScriptLoader::createScripts(
        const std::vector<ScriptData>& scripts) const
    {
        std::vector<script::script_id> registeredIds;

        for (auto& data : scripts) {
            for (auto scriptId : createScript(data)) {
                registeredIds.push_back(scriptId);
            }
        }

        return registeredIds;
    }

    std::vector<script::script_id> ScriptLoader::createScript(
        const ScriptData& data) const
    {
        if (!data.enabled) return {};

        std::vector<script::script_id> registeredIds;
        std::vector<script::ScriptFile> scriptFiles;

        if (!data.path.empty()) {
            auto& scriptFile = scriptFiles.emplace_back(
                false,
                data.type,
                data.path,
                readFile(data.path));
        }
        if (!data.script.empty()) {
            auto& scriptFile = scriptFiles.emplace_back(
                true,
                data.type,
                data.path,
                data.script);
        }

        for (const auto& scriptFile : scriptFiles) {
            if (scriptFile.m_source.empty()) continue;

            auto scriptId = script::ScriptSystem::get().registerScript(scriptFile);
            registeredIds.push_back(scriptId);
        }

        return registeredIds;
    }

    void ScriptLoader::bindTypeScripts(
        pool::TypeHandle handle,
        const std::vector<script::script_id>& scriptIds) const
    {
        const auto& assets = Assets::get();
        if (!assets.useScript) return;

        for (auto scriptId : scriptIds)
        {
            event::Event evt { event::Type::script_type_bind };
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
