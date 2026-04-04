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

        std::vector<AddonData> addons;
    };
}
