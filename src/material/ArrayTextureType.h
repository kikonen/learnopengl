#pragma once

#include <type_traits>
#include <stdint.h>

namespace material
{
    enum class ArrayTextureType : std::underlying_type_t<std::byte>
    {
        none,
        srgb,
        data,
        normal,
        displacement,
        dudv,
        noise,
        height,
        font_atlas,
        dynamic
    };
}
