#pragma once

#include <string>
#include <array>

#include <stdint.h>

#include "TextureSpec.h"

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
        std::string name;
        int uniformId;
        int channels;
        int size;
        int maxLayers;
        bool is16Bit;
        // If true treat channels == 1 as RGB instead RED
        // => i.e. color instead of "data"
        bool grayScale;
        bool gammaCorrect;
        bool hdri;
        material::TextureSpec spec;
        std::array<PixelType, 2> pixels { PixelType::none, PixelType::none };
    };
}
