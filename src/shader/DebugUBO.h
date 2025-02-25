#pragma once

#include "kigl/kigl.h"

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
#pragma pack(push, 1)
struct DebugUBO {
    int u_entityId;
    int u_boneIndex;

    int u_boneWeight; // bool as 4 bytes

    int u_normalMapEnabled; // bool as 4 bytes

    float u_parallaxDepth;
    int u_parallaxMethod;

    int u_wireframeOnly;
    float u_wireframeLineWidth;
    glm::vec3 u_wireframeLineColor;
};
#pragma pack(pop)
