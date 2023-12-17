#pragma once

#include <vector>
#include <string>

namespace loader
{
    struct ScriptData {
        bool enabled{ false };
        std::string path;
        std::string script;
    };

    struct ScriptEngineData {
        bool enabled{ false };
        std::vector<ScriptData> scripts;
    };
}
