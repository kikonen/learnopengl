#pragma once

#include <string>
#include <vector>

#include "ClipData.h"

namespace loader {
    struct AnimationData {
        bool enabled{ false };
        std::string name;
        std::string path;
        std::vector<ClipData> clips;
    };
}
