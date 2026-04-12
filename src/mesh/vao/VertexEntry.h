#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

#include "mesh/Vertex.h"

#include "size.h"

namespace mesh {
    struct VertexEntry {
        VertexEntry()
            : u_x{ 0 }, u_y{ 0 }, u_z{ 0 },
            u_texCoord{ 0, 0 },
            u_normal{ 0, 0, 0 },
            u_tangent{ 0, 0, 0, 1 }
        {}

        VertexEntry(const mesh::Vertex& v)
            : u_x{ v.pos.x },
            u_y{ v.pos.y },
            u_z{ v.pos.z },
            u_texCoord{ v.texCoord },
            u_normal{ v.normal },
            u_tangent{ v.tangent.x, v.tangent.y, v.tangent.z, calculateTangentW(v) }
        {}

        // NOTE KI kigl::UV16 accuracy was not enough in all cases
        // backpack, pinetree, ...
        //kigl::UV16 u_texCoord;
        glm::vec2 u_texCoord;

        float u_x;
        float u_y;
        float u_z;

#ifdef VAO_USE_NORMAL_VEC3_16N
        kigl::VEC3_16N u_normal;
        int16_t u_normal_pad{ 0 };      // 2 bytes → aligns to 8
        kigl::VEC4_16N u_tangent;
#elif VAO_USE_NORMAL_VEC10
        kigl::VEC10 u_normal;
        kigl::VEC10 u_tangent;
#else
        glm::vec3 u_normal;
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
}
