#pragma once

#include <string>

#include "asset/CustomMaterial.h"

namespace loader {
    enum class CustomMaterialType {
        none,
        text,
        skybox,
    };

    struct CustomMaterialData {
        CustomMaterialType type{ CustomMaterialType::none };

        std::string fontName;
        float fontSize;
    };
}
