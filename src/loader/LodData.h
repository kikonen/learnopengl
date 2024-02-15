#pragma once

#include <string>

#include "MaterialData.h"

namespace loader {
    struct LodData {
        float distance{ 0 };

        std::string meshPath;
        std::string meshName;

        std::string materialName;
        MaterialData materialModifiers;
    };
}
