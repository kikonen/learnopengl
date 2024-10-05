#pragma once

#include <string>
#include <type_traits>

#include "material/CustomMaterial.h"

#include "MaterialData.h"

namespace loader {
    enum class CustomMaterialType : std::underlying_type_t<std::byte> {
        none,
        skybox,
    };

    struct CustomMaterialData {
        CustomMaterialType type{ CustomMaterialType::none };

        MaterialData materialData;
    };
}
