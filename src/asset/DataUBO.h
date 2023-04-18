#pragma once

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct DataUBO {
    glm::vec3 u_viewPos;
    int pad_u_viewPos;

    glm::vec3 u_viewFront;
    int pad_u_viewFront;

    glm::vec3 u_viewUp;
    int pad_u_viewUp;

    glm::vec3 u_viewRight;
    int pad_u_viewRight;

    // NOTE KI std140
    // "Both the size and alignment are twice"
    // "the size of the underlying scalar type."
    glm::vec2 u_resolution;
    int u_cubeMapExist; // bool as 4 bytes
    int u_frustumVisual; // bool as 4 bytes

    glm::vec4 u_fogColor;

    float u_fogStart;
    float u_fogEnd;
    float u_fogRatio;
    float u_time;

    int u_shadowCount;
    int pad3_1;
    int pad3_2;
    int pad3_3;

    // From *camera* view (not shadow view)
    // NOTE KI std410 arrays are glm::vec4 *alignment* per item
    glm::vec4 u_shadowPlanes[MAX_SHADOW_MAP_COUNT + 1];
};
#pragma pack(pop)
