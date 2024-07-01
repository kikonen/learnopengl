#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"
#include "kigl/kigl.h"

//
// SSBO entry
//
#pragma pack(push, 1)
struct ParticleSSBO {
    // xyz = pos, a = scale
    float u_x;
    float u_y;
    float u_z;
    float u_scale;

    GLuint u_materialIndex;
    GLuint u_spriteIndex;

    //int pad1;
    //int pad2;
};
#pragma pack(pop)
