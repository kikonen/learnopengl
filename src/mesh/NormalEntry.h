#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct NormalEntry {
        NormalEntry()
            : normal{ {0.f, 0.f, 0.f} },
            tangent{ {0.f, 0.f, 0.f} }
        {}

        NormalEntry(
            const glm::vec3& a_normal,
            const glm::vec3& a_tangent)
            : normal{ a_normal },
            tangent{ a_tangent }
        {}

        kigl::VEC10 normal;
        kigl::VEC10 tangent;
    };
#pragma pack(pop)
}
