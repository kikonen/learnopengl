#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct NormalEntry {
        NormalEntry()
            : u_normal{ 0.f },
            u_tangent{ 0.f }
        {}

        NormalEntry(
            const glm::vec3& a_normal,
            const glm::vec3& a_tangent)
            : u_normal{ a_normal },
            u_tangent{ a_tangent }
        {}

        kigl::VEC10 u_normal;
        kigl::VEC10 u_tangent;
    };
#pragma pack(pop)
}
