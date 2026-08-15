#pragma once

#include <string>

namespace loader {
    struct ClipData {
        bool enabled{ false };
        // NOTE KI if "name" is not set then refers to clips embedded in mesh file itself
        std::string name;
        std::string clip;

        std::string getUniqueName(const std::string& animationPrefix) const
        {
            if (animationPrefix.empty()) {
                // NOTE KI "master" refers to animations embedded in same file as mesh itself
                return "master:" + clip;
            }
            return animationPrefix + ":" + clip;
        }
    };
}
