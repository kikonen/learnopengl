#pragma once

#include <string>

#include "TextureSpec.h"

namespace material
{
    struct ArrayTextureInfo
    {
        std::string name;
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
    };
}
