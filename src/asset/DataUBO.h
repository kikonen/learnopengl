#pragma once

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
struct DataUBO {
    glm::vec3 u_viewPos;
    float u_time;

    glm::vec2 u_resolution;
    int pad1;
    int pad2;

    glm::vec4 u_fogColor;
    float u_fogStart;
    float u_fogEnd;

    int pad3;
    int pad4;
};
