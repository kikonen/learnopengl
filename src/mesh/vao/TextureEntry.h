#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct TextureEntry {
        TextureEntry()
            : u_texCoord{ 0.f, 0.f }
        {}

        TextureEntry(
            const glm::vec2& a_texCoord)
            : u_texCoord{ a_texCoord }
        {}

        kigl::UV16 u_texCoord;
    };
#pragma pack(pop)
}
