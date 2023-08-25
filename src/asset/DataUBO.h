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

    glm::vec3 u_mainViewPos;
    int pad_u_mainViewPos;

    glm::vec3 u_mainViewFront;
    int pad_u_mainViewFront;

    glm::vec3 u_mainViewUp;
    int pad_u_mainViewUp;

    glm::vec3 u_mainViewRight;
    int pad_u_mainViewRight;

    glm::vec4 u_fogColor;

    // NOTE KI std140
    // "Both the size and alignment are twice"
    // "the size of the underlying scalar type."
    glm::vec2 u_screenResolution;
    int u_cubeMapExist; // bool as 4 bytes
    int u_frustumVisual; // bool as 4 bytes

    float u_fogStart;
    float u_fogEnd;
    float u_fogDensity;

    float u_hdrGamma;
    float u_hdrExposure;
    float u_effectBloomExposure;

    float u_time;
    int u_shadowCount;
    //int pad3_1;
    //int pad3_2;
    //int pad3_3;

    // From *camera* view (not shadow view)
    // NOTE KI std410 arrays are glm::vec4 *alignment* per item
    glm::vec4 u_shadowPlanes[MAX_SHADOW_MAP_COUNT + 1];
};
#pragma pack(pop)
