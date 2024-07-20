#include "ScriptLoader.h"

#include "util/Util.h"

#include "model/Node.h"

#include "script/ScriptEngine.h"
#include "event/Dispatcher.h"
#include "registry/Registry.h"

#include "loader/document.h"
#include "loader_util.h"

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
                loadScripts(v, data.scripts);
            }
            else {
                reportUnknown("script_engine_entry", k, v);
            }
        }
    }

    void ScriptLoader::loadScripts(
        const loader::DocNode& node,
        std::vector<ScriptData>& scripts) const
    {
        for (const auto& entry : node.getNodes()) {
            ScriptData& data = scripts.emplace_back();
            loadScript(entry, data, false);
        }
    }

    void ScriptLoader::loadScript(
        const loader::DocNode& node,
        ScriptData& data,
        bool forceFile) const
    {
        data.enabled = true;

        if (node.isScalar()) {
            std::string filename = readString(node) + ".lua";
            if (forceFile || fileExists(filename)) {
                data.script = readFile(filename);
            }
            else {
                data.script = readString(node);
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
        const ki::node_id rootId,
        const ScriptEngineData& data)
    {
        if (!data.enabled) return;
        createScripts(rootId, 0, data.scripts);
    }

    void ScriptLoader::createScripts(
        const ki::node_id rootId,
        ki::node_id nodeId,
        const std::vector<ScriptData>& scripts) const
    {
        for (auto& data : scripts) {
            createScript(rootId, 0, data);
        }
    }

    void ScriptLoader::createScript(
        const ki::node_id rootId,
        ki::node_id nodeId,
        const ScriptData& data) const
    {
        if (!data.enabled) return;

        std::vector<std::string> scripts;

        if (!data.path.empty()) {
            std::string filename = data.path + ".lua";
            scripts.push_back(readFile(filename));
        }
        scripts.push_back(data.script);

        for (const auto& script : scripts) {
            if (script.empty()) continue;

            auto scriptId = script::ScriptEngine::get().registerScript(script);
            {
                event::Event evt { event::Type::script_bind };
                auto& body = evt.body.script = {
                    .target = nodeId,
                    .id = scriptId,
                };
                m_dispatcher->send(evt);
            }

            if (nodeId == 0) {
                event::Event evt { event::Type::script_run };
                auto& body = evt.body.script = {
                    .target = 0,
                    .id = scriptId,
                };
                m_dispatcher->send(evt);
            }
        }
    }
}
