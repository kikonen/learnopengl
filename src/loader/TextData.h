#pragma once

#include <string>

#include "MaterialData.h"
#include "FontData.h"

namespace loader {
    struct TextData {
        bool enabled{ false };

        std::string text;

        MaterialData materialData;
        FontData fontData;
    };
}
