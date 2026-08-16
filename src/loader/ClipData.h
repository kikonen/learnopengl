#pragma once

#include <string>

namespace loader {
    struct ClipData {
        bool enabled{ false };
        // name for SID to reference clip in system
        std::string name;
        // name of clip in imported animation file
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
