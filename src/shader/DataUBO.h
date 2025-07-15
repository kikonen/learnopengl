#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct DataUBO {
    glm::vec4 u_fogColor;

    GLuint u_selectionMaterialIndex;
    GLuint u_tagMaterialIndex;

    int u_cubeMapEnabled; // bool as 4 bytes
    int u_skyboxExist; // bool as 4 bytes

    int u_environmentMapExist; // bool as 4 bytes

    int u_shadowVisual; // bool as 4 bytes
    int u_forceLineMode; // bool as 4 bytes

    float u_fogStart;
    float u_fogEnd;
    float u_fogDensity;

    float u_effectBloomThresHold;

    float u_gammaCorrect;
    float u_hdrExposure;

    float u_time;
    int u_frame;
    int u_shadowCount;

    glm::vec3 u_ssaoSamples[64];

    // From *camera* view (not shadow view)
    // NOTE KI std410 arrays are glm::vec4 *alignment* per item
    float u_shadowCascade_0;
    float u_shadowCascade_1;
    float u_shadowCascade_2;
    float u_shadowCascade_3;

    //int pad1;
    //int pad2;
};
#pragma pack(pop)
