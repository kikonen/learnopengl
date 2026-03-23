#pragma once

#include <string>
#include <vector>
#include <stdint.h>

#include <glm/glm.hpp>

#include "AddonData.h"

namespace loader
{
    struct AddonSelectorData
    {
        bool enabled{ true };

        uint32_t seed{ 0 };
        glm::uvec2 range{ 1, 1 };

        std::vector<AddonData> addons;
    };
}
