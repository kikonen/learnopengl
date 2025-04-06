#pragma once

#include <vector>
#include <string>

#include "script/ScriptType.h"

namespace loader
{
    struct ScriptData {
        bool enabled{ false };
        script::ScriptType type{ script::ScriptType::none };
        std::string path;
        std::string script;
    };

    struct ScriptEngineData {
        bool enabled{ false };
        std::vector<ScriptData> scripts;
    };
}
