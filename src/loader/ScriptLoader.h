#pragma once

#include "ki/size.h"

#include "pool/NodeHandle.h"

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
            const loader::DocNode& node,
            ScriptEngineData& data) const;

        void loadScripts(
            const loader::DocNode& node,
            std::vector<ScriptData>& scripts) const;

        void loadScript(
            const loader::DocNode& node,
            ScriptData& data,
            bool forceFile) const;

        void createScriptEngine(
            const ScriptEngineData& data);

        void createScripts(
            pool::NodeHandle handle,
            const std::vector<ScriptData>& scripts) const;

        void createScript(
            pool::NodeHandle handle,
            const ScriptData& data) const;
    };
}
