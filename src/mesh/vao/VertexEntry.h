#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

#include "mesh/Vertex.h"

namespace mesh {
#pragma pack(push, 1)
    struct VertexEntry {
        VertexEntry() {}

        VertexEntry(const mesh::Vertex& v)
            : u_pos(v.pos),
            u_texture(v.texture),
            u_normal(v.normal),
            u_tangent(v.tangent)
        {}

        glm::vec3 u_pos;

        //kigl::UV16 texCoord;
        glm::vec2 u_texture;

        kigl::VEC10 u_normal;
        kigl::VEC10 u_tangent;
    };
#pragma pack(pop)
}
