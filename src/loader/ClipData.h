#pragma once

#include <string>

namespace loader {
    struct ClipData {
        bool enabled{ false };
        std::string name;
        std::string clip;

        std::string getUniqueName(const std::string& prefix) const
        {
            if (prefix.empty()) {
                return "master:" + clip;
            }
            return prefix + ":" + clip;
        }
    };
}
