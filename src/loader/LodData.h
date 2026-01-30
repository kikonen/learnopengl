#pragma once

#include <vector>
#include <stdint.h>

#include "FlagContainer.h"

#include "MaterialData.h"

namespace loader {
    inline const std::string LOD_ALIAS_ANY = "*";

    struct LodData {
        std::string name;

        int8_t priority{ 0 };

        float minDistance{ 0 };
        float maxDistance{ 0 };

        std::vector<MaterialData> materialModifiers;

        loader::FlagContainer meshFlags;

        inline bool isAny() const noexcept
        {
            return name == LOD_ALIAS_ANY;
        }

        inline bool match(const std::string dstName) const noexcept
        {
            return name == dstName;
        }
    };
}
