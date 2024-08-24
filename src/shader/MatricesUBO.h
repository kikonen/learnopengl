#pragma once

#include <glm/glm.hpp>

#include "Shader.h"


// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct MatricesUBO {
    glm::mat4 u_mainProjected{ 1.0f };

    glm::mat4 u_projected{ 1.0f };

    glm::mat4 u_projection{ 1.0f };
    glm::mat4 u_invProjection{ 1.0f };

    glm::mat4 u_view{ 1.0f };
    glm::mat4 u_invView{ 1.0f };

    glm::mat4 u_viewSkybox{ 1.0f };

    // NOTE KI calculated by shadow calculation
    glm::mat4 u_shadow[MAX_SHADOW_MAP_COUNT_ABS];

    // top, bottom, left, right, near, far
    glm::vec4 u_frustumPlanes[6];
};
#pragma pack(pop)
