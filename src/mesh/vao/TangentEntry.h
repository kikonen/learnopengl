#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct TangentEntry {
        TangentEntry()
            : u_tangent{ 0.f }
        {
        }

        TangentEntry(
            const glm::vec3& a_tangent)
            : u_tangent{ a_tangent }
        {
        }

        kigl::VEC10 u_tangent;
        //kigl::VEC3_16 u_tangent;
    };
#pragma pack(pop)
}
