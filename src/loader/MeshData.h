#pragma once

#include <string>
#include <vector>

#include "AnimationData.h"
#include "MaterialData.h"

namespace loader {
    struct MeshData {
        std::string baseDir;

        int16_t level{ 0 };
        std::string meshPath;
        std::vector<MaterialData> materials;
        std::vector<AnimationData> animations;
    };
}
