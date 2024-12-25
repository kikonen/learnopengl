#pragma once

#include <string>

#include "MaterialData.h"
#include "FontData.h"

#include "text/Align.h"

namespace loader {
    struct TextData {
        bool enabled{ false };

        std::string text;

        glm::vec2 pivot{ 0.f };
        text::Align alignHorizontal{ text::Align::left };
        text::Align alignVertical{ text::Align::top };

        MaterialData materialData;
        FontData fontData;
    };
}
