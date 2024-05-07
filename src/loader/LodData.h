#pragma once

#include <string>
#include <vector>

#include "MaterialReference.h"

namespace loader {
    struct LodData {
        float distance{ 0 };

        std::string meshPath;

        std::vector<MaterialReference> materialReferences;
    };
}
