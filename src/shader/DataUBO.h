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

    GLuint u_particleBaseIndex;
    GLuint u_boneBaseIndex;
    GLuint u_socketBaseIndex;

    float u_fogStart;
    float u_fogEnd;
    float u_fogDensity;

    float u_effectOitMinBlendThreshold;
    float u_effectOitMaxBlendThreshold;

    float u_effectBloomThresHold;

    float u_gammaCorrect;
    float u_hdrExposure;

    float u_time;
    int u_frame;

    //int pad1;
    //int pad2;
    //int pad3;

    glm::vec3 u_ssaoSamples[64];
};
#pragma pack(pop)
