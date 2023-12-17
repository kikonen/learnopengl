#pragma once

#include "ki/size.h"

#include "BaseLoader.h"

#include "ScriptData.h"

namespace script
{
    class ScriptEngine;
}

namespace loader {
    class ScriptLoader : public BaseLoader
    {
    public:
        ScriptLoader(
            Context ctx);

        void loadScriptEngine(
            const YAML::Node& node,
            ScriptEngineData& data) const;

        void loadScripts(
            const YAML::Node& node,
            std::vector<ScriptData>& scripts) const;

        void loadScript(

            const YAML::Node& node,
            ScriptData& data) const;

        void createScriptEngine(
            const uuids::uuid& rootId,
            const ScriptEngineData& data);

        void createScripts(
            const uuids::uuid& rootId,
            ki::object_id nodeId,
            const std::vector<ScriptData>& scripts) const;

        void createScript(
            const uuids::uuid& rootId,
            ki::object_id nodeId,
            const ScriptData& data) const;
    };
}
