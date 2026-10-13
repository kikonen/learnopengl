#pragma once

#include <string>
#include <array>

#include <stdint.h>

#include "TextureSpec.h"
#include "ArrayTextureType.h"

namespace material
{
    enum PixelType : std::underlying_type_t<std::byte>
    {
        none,
        magenta,
        black,
        white,
        normal
    };

    struct ArrayTextureInfo
    {
        material::ArrayTextureType type;
        int uniformId;
        int channels;
        int size;
        int maxLayers;
        bool is16Bit;
        // If true treat channels == 1 as RGB instead RED
        // => i.e. color instead of "data"
        bool grayScale;
        // Data is in srgb format
        bool gammaCorrect;
        bool hdri;
        material::TextureSpec spec;
        std::array<material::PixelType, 2> pixels {
            material::PixelType::none,
            material::PixelType::none
        };
    };
}
