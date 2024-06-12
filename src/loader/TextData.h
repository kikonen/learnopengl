#pragma once

#include <string>

#include "MaterialData.h"
#include "FontData.h"

namespace loader {
    struct TextData {
        bool enabled{ false };

        std::string text;
        std::string font;

        MaterialData materialData;
        FontData fontData;
    };
}
