#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"
#include "kigl/kigl.h"

//
// SSBO entry
//
#pragma pack(push, 1)
struct ParticleSSBO {
    glm::vec3 u_pos{ 0.f };

    float u_scale{ 1.f };

    GLint u_materialIndex{ 0 };

    glm::uvec2 m_spriteIndex{ 0 };
};
#pragma pack(pop)
