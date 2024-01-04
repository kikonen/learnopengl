#pragma once

#include <string>

#include <glm/glm.hpp>

#include "text/size.h"

namespace loader {
    struct FontData {
        text::font_id id;

        std::string name;

        std::string path{ "fonts/Vera.ttf" };
        float size{ 12.f };

        glm::uvec2 atlasSize{ 512 };
    };

}
