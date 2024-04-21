#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct TextureEntry {
        TextureEntry()
            : texCoord{ 0.f, 0.f }
        {}

        TextureEntry(
            const glm::vec2& a_texCoord)
            : texCoord{ a_texCoord }
        {}

        //kigl::UV16 texCoord;
        glm::vec2 texCoord;
    };
#pragma pack(pop)
}
