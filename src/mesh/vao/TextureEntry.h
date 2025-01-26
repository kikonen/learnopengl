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

        // NOTE KI kigl::UV16 accuracy was not enough in all cases
        // backpack, pinetree, ...
        //kigl::UV16 u_texCoord;
        glm::vec2 u_texCoord;
    };
#pragma pack(pop)
}
