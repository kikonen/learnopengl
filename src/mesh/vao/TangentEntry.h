#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

#include "mesh/Vertex.h"

#include "size.h"

namespace mesh {
#pragma pack(push, 1)
    struct TangentEntry {
        TangentEntry()
            : u_tangent{ 0.f, 0.f, 0.f, 1.f }
        {
        }

        TangentEntry(const mesh::Vertex& v)
            : u_tangent{ v.tangent.x, v.tangent.y, v.tangent.z, calculateTangentW(v) }
        {
        }

#ifdef VAO_USE_NORMAL_VEC3_16N
        kigl::VEC4_16N u_tangent;
#elif VAO_USE_NORMAL_VEC10
        kigl::VEC10 u_tangent;
#else
        glm::vec4 u_tangent;
#endif

    private:
        static float calculateTangentW(const Vertex& v)
        {
            const auto cross = glm::cross(v.normal, v.tangent);
            // degenerate mesh: zero-length tangent or parallel to normal
            if (glm::dot(cross, cross) < 1e-6f) return 1.f;
            return glm::dot(cross, v.bitangent) < 0.f ? -1.f : 1.f;
        }
    };
#pragma pack(pop)
}
