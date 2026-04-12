#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

#include "size.h"

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

#ifdef VAO_USE_NORMAL_VEC3_16N
        kigl::VEC3_16N u_normal;
#elif VAO_USE_NORMAL_VEC10
        kigl::VEC10 u_normal;
#else
        glm::vec3 u_normal;
#endif

    };
#pragma pack(pop)
}
