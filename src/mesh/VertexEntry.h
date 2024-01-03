#pragma once

#include "glm/glm.hpp"

#include "kigl/kigl.h"

namespace mesh {
#pragma pack(push, 1)
    struct PositionEntry {
        glm::vec3 pos;
    };
#pragma pack(pop)

#pragma pack(push, 1)
    struct VertexEntry {
        kigl::VEC10 normal;
        kigl::VEC10 tangent;
        kigl::UV16 texCoord;
    };
#pragma pack(pop)
}
