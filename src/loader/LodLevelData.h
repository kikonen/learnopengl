#pragma once

#include "ki/size.h"

namespace loader {
    struct LodLevelData {
        std::vector<int> levels{ {0} };
        float distance{ 0 };

        uint8_t getLevelMask() const noexcept
        {
            uint8_t mask = 0;
            for (auto& level : levels) {
                mask |= 1 << level;
            }
            return mask;
        }

        float getDistance2() const noexcept
        {
            return distance * distance;
        }
    };
}
