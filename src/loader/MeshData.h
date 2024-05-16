#pragma once

#include <string>
#include <vector>

#include "AnimationData.h"
#include "MaterialReference.h"

namespace loader {
    struct MeshData {
        int16_t level{ 0 };
        std::string meshPath;
        std::vector<MaterialReference> materialReferences;
        std::vector<AnimationData> animations;
    };
}
