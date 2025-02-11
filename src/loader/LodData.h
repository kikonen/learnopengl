#pragma once

#include <vector>
#include <stdint.h>

#include "FlagContainer.h"

#include "MaterialData.h"

namespace loader {
    struct LodData {
        std::string name;

        int8_t priority{ 0 };

        float minDistance{ 0 };
        float maxDistance{ 0 };

        std::vector<MaterialData> materialModifiers;

        loader::FlagContainer meshFlags;
    };
}
