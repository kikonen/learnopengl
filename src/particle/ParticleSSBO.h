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

    GLint u_materialIndex{ 0 };

    glm::uvec2 m_spriteIndex{ 0 };
};
#pragma pack(pop)
