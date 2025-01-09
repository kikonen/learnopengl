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
        int padding{ 8 };

        glm::uvec2 atlasSize{ 1024 };
    };

}
