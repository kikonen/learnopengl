#pragma once

#include <string>

#include "asset/CustomMaterial.h"

namespace loader {
    enum class CustomMaterialType : std::underlying_type_t<std::byte> {
        none,
        text,
        skybox,
    };

    struct CustomMaterialData {
        CustomMaterialType type{ CustomMaterialType::none };

        std::string font;
    };
}
