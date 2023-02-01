#pragma once

#include <glm/glm.hpp>

// NOTE KI align 16 for UBO struct
struct DataUBO {
    glm::vec3 u_viewPos;
    int pad5;

    glm::vec3 u_viewFront;
    int pad6;

    glm::vec3 u_viewUp;
    int pad7;

    glm::vec3 u_viewRight;

    float u_time;

    glm::vec2 u_resolution;
    int pad1;
    int pad2;

    glm::vec4 u_fogColor;
    float u_fogStart;
    float u_fogEnd;

    bool u_cubeMapExist;

    //int pad3;
    int pad4;

};
