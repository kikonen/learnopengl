#pragma once

#include <glm/glm.hpp>

#include "asset/Shader.h"


// NOTE KI align 16 for UBO struct
struct MatricesUBO {
    glm::mat4 u_projected{ 1.0f };
    glm::mat4 u_projection{ 1.0f };
    glm::mat4 u_view{ 1.0f };
    glm::mat4 u_viewSkybox{ 1.0f };

    // NOTE KI calculated by shadow calculation
    glm::mat4 u_shadow[SHADOW_MAP_COUNT];

    // NOTE KI shadow calculation only
    glm::mat4 u_shadowProjected[SHADOW_MAP_COUNT];

    // From *view* point of view (not shadow rendering)
    float u_shadowEndClipZ[SHADOW_MAP_COUNT];
    float pad1;
    //glm::vec4 pad1;
    glm::vec4 pad2;
    glm::vec4 pad3;
};
