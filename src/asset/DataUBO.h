#pragma once

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
struct DataUBO {
    glm::vec3 u_viewPos;
    int pad_u_viewPos;

    glm::vec3 u_viewFront;
    int pad_u_viewFront;

    glm::vec3 u_viewUp;
    int pad_u_viewUp;

    glm::vec3 u_viewRight;

    float u_time;

    glm::vec2 u_resolution;
    int pad_u_resolution_1;
    int pad_u_resolution_2;

    glm::vec4 u_fogColor;
    float u_fogStart;
    float u_fogEnd;
    float u_fogRatio;

    bool u_cubeMapExist;

    int u_shadowCount;
    int pad3_1;
    int pad3_2;
    int pad3_3;

    // From *camera* view (not shadow view)
    // NOTE KI std410 arrays are glm::vec4 *alignment* per item
    glm::vec4 u_shadowPlanes[MAX_SHADOW_MAP_COUNT + 1];

    //int pad3;
    //int pad4;
};
