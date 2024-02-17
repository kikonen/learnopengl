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

    int pad1;
    int pad2;
};
#pragma pack(pop)
