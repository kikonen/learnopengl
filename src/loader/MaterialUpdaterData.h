#pragma once

#include <string>
#include <type_traits>

#include "MaterialData.h"

namespace loader {
    enum class MaterialUpdaterType : std::underlying_type_t<std::byte> {
        none,
        shader,
        font_atlas,
    };

    struct MaterialUpdaterData
    {
        MaterialUpdaterType type{ MaterialUpdaterType::none };

        std::string id;

        glm::vec2 size{ 512.f, 512.f };
        int frameSkip{ 1 };

        MaterialData materialData;
    };
}
