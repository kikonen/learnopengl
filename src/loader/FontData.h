#pragma once

#include <string>

#include <glm/glm.hpp>

#include "text/size.h"

namespace loader {
    struct FontData {
        text::font_id id;

        std::string name;

        std::string path{ "fonts/Vera.ttf" };
        float size{ 64.f };
        int padding{ 64 };

        glm::uvec2 atlasSize{ 2048, 2048 };
    };

}
