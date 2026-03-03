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
        uint32_t seed{ 0 };
        glm::uvec2 range{ 1, 1 };
    };
}
