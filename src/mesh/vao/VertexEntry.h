#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

#include "mesh/Vertex.h"

namespace mesh {
#pragma pack(push, 1)
    struct VertexEntry {
        VertexEntry()
            : u_x{ 0 }, u_y{ 0 }, u_z{ 0 },
            u_texCoord{ 0, 0 },
            u_normal{ 0, 0, 0 },
            u_tangent{ 0, 0, 0 }
        {}

        VertexEntry(const mesh::Vertex& v)
            : u_x{ v.pos.x },
            u_y{ v.pos.y },
            u_z{ v.pos.z },
            u_texCoord{ v.texCoord },
            u_normal{ v.normal },
            u_tangent{ v.tangent }
        {}

        float u_x;
        float u_y;
        float u_z;

        //kigl::VEC10 u_normal;
        kigl::VEC3_16 u_normal;

        //kigl::VEC10 u_tangent;
        kigl::VEC3_16 u_tangent;

        // NOTE KI kigl::UV16 accuracy was not enough in all cases
        // backpack, pinetree, ...
        //kigl::UV16 u_texCoord;
        glm::vec2 u_texCoord;
    };
#pragma pack(pop)
}
