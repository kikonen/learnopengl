#pragma once

#include <string>
#include <vector>

#include "ClipData.h"

namespace loader {
    struct AnimationData {
        bool enabled{ false };
        // NOTE KI name is empty for animation clips embedded in mesh file itself
        // => if path is set then name MUST be also set
        std::string name;
        // NOTE KI path is empty for animation clips embedded in mesh file itself 
        std::string path;
        std::vector<ClipData> clips;
    };
}
