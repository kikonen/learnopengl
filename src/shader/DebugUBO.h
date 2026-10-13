#pragma once

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct DebugUBO {
    glm::vec3 u_wireframeLineColor; // vec3 as 4 floats
    glm::vec3 u_skyboxColor; // vec3 as 4 floats
    glm::vec3 u_ssaoBaseColor; // vec3 as 4 floats

    int32_t u_wireframeOnly; // bool as 4 bytes
    float u_wireframeLineWidth;

    int32_t u_entityId;
    int32_t u_jointIndex;

    int32_t u_jointWeight; // bool as 4 bytes

    int32_t u_lightEnabled; // bool as 4 bytes
    int32_t u_normalMapEnabled; // bool as 4 bytes

    int32_t u_skyboxColorEnabled; // bool as 4 bytes

    int32_t u_ssaoEnabled; // bool as 4 bytes
    int32_t u_ssaoBaseColorEnabled; // bool as 4 bytes

    int32_t u_parallaxEnabled; // bool as 4 bytes
    float u_parallaxDepth;
    int32_t u_parallaxMethod;

    uint32_t pad1;
    uint32_t pad2;
    uint32_t pad3;
};
#pragma pack(pop)
