#pragma once

#include "glm/glm.hpp"

#include "ki/GL.h"

#pragma pack(push, 1)
struct PositionEntry {
    glm::vec3 pos;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct VertexEntry {
    ki::VEC10 normal;
    ki::VEC10 tangent;
    ki::UV16 texCoord;
};
#pragma pack(pop)
