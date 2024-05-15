#pragma once

#include <string>
#include <vector>

#include "MaterialReference.h"

namespace loader {
    struct LodData {
        uint16_t level{ 0 };
        float distance{ 0 };

        std::string meshPath;

        std::vector<MaterialReference> materialReferences;
    };
}
