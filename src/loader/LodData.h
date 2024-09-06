#pragma once

#include <stdint.h>

#include "FlagContainer.h"

namespace loader {
    struct LodData {
        std::string name;

        std::vector<int> levels{ {0} };
        int8_t priority{ 0 };

        float distance{ 0 };

        loader::FlagContainer meshFlags;

        uint8_t getLevelMask() const noexcept
        {
            uint8_t mask = 0;
            for (auto& level : levels) {
                if (level < 0) continue;
                mask |= (1 << level);
            }
            return mask;
        }
    };
}
