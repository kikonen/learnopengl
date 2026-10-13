#pragma once

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct DataUBO {
    glm::vec4 u_fogColor;

    uint32_t u_selectionMaterialIndex;
    uint32_t u_tagMaterialIndex;
    uint32_t u_wireframeMaterialIndex;

    int32_t u_waterCausticEnabled; // bool as 4 bytes
    uint32_t u_waterCausticMaterialIndex;
    float u_waterCausticIntensity;
    float u_waterCausticWorldLevel;
    float u_waterCausticScale;

    int32_t u_cubeMapEnabled; // bool as 4 bytes
    int32_t u_skyboxExist; // bool as 4 bytes

    int32_t  u_environmentMapExist; // bool as 4 bytes

    int32_t u_shadowVisual; // bool as 4 bytes
    int32_t u_forceLineMode; // bool as 4 bytes

    float u_fogStart;
    float u_fogEnd;
    float u_fogDensity;

    float u_effectOitMinBlendThreshold;
    float u_effectOitMaxBlendThreshold;

    float u_effectBloomThresHold;

    float u_gammaCorrect;
    float u_hdrExposure;

    float u_time;
    int32_t u_frame;

    float u_worldTime;

    //int pad1;
    //int pad2;
    //int pad3;

    glm::vec3 u_ssaoSamples[64];
};
#pragma pack(pop)
