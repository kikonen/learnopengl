#pragma once

#include <string>
#include <vector>
#include <stdint.h>

namespace loader
{
    struct AddonData
    {
        bool enabled{ true };

        std::string id;
        std::string group;

        // 0 == no randomization
        uint32_t seed{ 0 };

        // how many elements to select from group
        // min == [0, n]
        // max = [n, -1], -1 == group size
        glm::uvec2 range{ 1, 1 };
    };
}
