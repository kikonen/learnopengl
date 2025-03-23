#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct NormalEntry {
        NormalEntry()
            : u_normal{ 0.f }
        {}

        NormalEntry(
            const glm::vec3& a_normal)
            : u_normal{ a_normal }
        {}

        kigl::VEC10 u_normal;
        //kigl::VEC3_16 u_normal;
    };
#pragma pack(pop)
}
