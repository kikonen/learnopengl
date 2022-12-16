#pragma once

#include "glm/glm.hpp"

#include "ki/GL.h"

#pragma pack(push, 1)
struct VertexEntry {
    glm::vec3 pos;
    ki::VEC10 normal;
    ki::VEC10 tangent;
    ki::UV16 texCoords;
};
#pragma pack(pop)
