#pragma once

#include <string>

namespace loader {
    struct AnimationData {
        bool enabled{ false };
        std::string name;
        std::string path;
    };
}
