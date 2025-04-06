#pragma once

#include "ki/size.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "script/size.h"

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
            std::vector<ScriptData>& scripts,
            bool forceFile) const;

        void loadScript(
            const loader::DocNode& node,
            ScriptData& data,
            bool forceFile) const;

        void createScriptEngine(
            const ScriptEngineData& data);

        std::vector<script::script_id> createScripts(
            const std::vector<ScriptData>& scripts) const;

        std::vector<script::script_id> createScript(
            const ScriptData& data) const;

        void bindTypeScripts(
            pool::TypeHandle handle,
            const std::vector<script::script_id>& scriptIds) const;

        void runGlobalScripts(
            const std::vector<script::script_id>& scriptIds) const;

        std::string resolveScriptPath(const std::string& str) const;
    };
}
