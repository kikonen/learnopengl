#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct VertexEntry {
        VertexEntry()
            : normal{ {0.f, 0.f, 0.f} },
            tangent{ {0.f, 0.f, 0.f} },
            texCoord{ {0.f, 0.f} }
        {}

        VertexEntry(
            const glm::vec3& a_normal,
            const glm::vec3& a_tangent,
            const glm::vec2& a_texCoord)
            : normal{ a_normal },
            tangent{ a_tangent },
            texCoord{ a_texCoord }
        {}

        kigl::VEC10 normal;
        kigl::VEC10 tangent;
        kigl::UV16 texCoord;
    };
#pragma pack(pop)
}
