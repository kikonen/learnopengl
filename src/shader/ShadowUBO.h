#pragma once

#include <glm/glm.hpp>

#include "Shader.h"

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct ShadowUBO {
    // NOTE KI calculated by shadow calculation
    glm::mat4 u_shadow[MAX_SHADOW_MAP_COUNT_ABS];

    int32_t u_shadowCount;

    // From *camera* view (not shadow view)
    // NOTE KI std410 arrays are glm::vec4 *alignment* per item
    float u_shadowCascade_0;
    float u_shadowCascade_1;
    float u_shadowCascade_2;
    float u_shadowCascade_3;

    uint32_t pad1;
    uint32_t pad2;
    uint32_t pad3;
};
#pragma pack(pop)
