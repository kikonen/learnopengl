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
    glm::vec4 u_pos_scale{ 0.f };

    GLuint u_materialIndex{ 0 };
    GLuint u_spriteIndex{ 0 };

    int pad1;
    int pad2;
};
#pragma pack(pop)
