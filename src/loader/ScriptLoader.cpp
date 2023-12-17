#include "ScriptLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "model/Node.h"

#include "script/ScriptEngine.h"
#include "event/Dispatcher.h"
#include "registry/Registry.h"


namespace loader {
    ScriptLoader::ScriptLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void ScriptLoader::loadScriptEngine(
        const YAML::Node& node,
        ScriptEngineData& data) const
    {
        data.enabled = true;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "xxenabled" || k == "xenabled") {
                // NOTE compat with old "disable" logic
                data.enabled = false;
            }
            else if (k == "scripts") {
                loadScripts(v, data.scripts);
            }
            else {
                reportUnknown("script_engine_entry", k, v);
            }
        }
    }

    void ScriptLoader::loadScripts(
        const YAML::Node& node,
        std::vector<ScriptData>& scripts) const
    {
        for (const auto& entry : node) {
            ScriptData& data = scripts.emplace_back();
            loadScript(entry, data);
        }
    }

    void ScriptLoader::loadScript(
        const YAML::Node& node,
        ScriptData& data) const
    {
        data.enabled = true;

        if (node.IsScalar()) {
            std::string filename = readString(node) + ".lua";
            data.script = readFile(filename);
            return;
        }

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "xxenabled" || k == "xenabled") {
                // NOTE compat with old "disable" logic
                data.enabled = false;
            }
            else if (k == "path") {
                data.path = readString(v);
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
        const uuids::uuid& rootId,
        const ScriptEngineData& data)
    {
        if (!data.enabled) return;
        createScripts(rootId, 0, data.scripts);
    }

    void ScriptLoader::createScripts(
        const uuids::uuid& rootId,
        ki::object_id nodeId,
        const std::vector<ScriptData>& scripts) const
    {
        for (auto& data : scripts) {
            createScript(rootId, 0, data);
        }
    }

    void ScriptLoader::createScript(
        const uuids::uuid& rootId,
        ki::object_id nodeId,
        const ScriptData& data) const
    {
        if (!data.enabled) return;

        if (!data.path.empty()) {
            std::string filename = data.path + ".lua";
            auto script = readFile(filename);

            //m_registry->m_
            //{
            //    event::Event evt { event::Type::script_add };
            //    auto& body = evt.body.script = {
            //        .script = script,
            //    };
            //    m_dispatcher->send(evt);
            //}
        }

        if (!data.script.empty()) {
            m_registry->m_scriptEngine;
        }
    }
}

