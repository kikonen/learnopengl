#pragma once

#include "kigl/kigl.h"

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct DebugUBO {
    glm::vec3 u_wireframeLineColor; // vec3 as 4 floats
    glm::vec3 u_skyboxColor; // vec3 as 4 floats
    glm::vec3 u_ssaoBaseColor; // vec3 as 4 floats

    int u_wireframeOnly; // bool as 4 bytes
    float u_wireframeLineWidth;

    int u_entityId;
    int u_jointIndex;

    int u_jointWeight; // bool as 4 bytes

    int u_lightEnabled; // bool as 4 bytes
    int u_normalMapEnabled; // bool as 4 bytes

    int u_skyboxColorEnabled; // bool as 4 bytes

    int u_ssaoEnabled; // bool as 4 bytes
    int u_ssaoBaseColorEnabled; // bool as 4 bytes

    float u_parallaxDepth;
    int u_parallaxMethod;

    //int pad1;
    //int pad2;
    //int pad3;
};
#pragma pack(pop)
