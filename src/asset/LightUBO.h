#pragma once

#include <glm/glm.hpp>

#include "ki/GL.h"

constexpr unsigned int MAX_LIGHT_COUNT = 128;


// NOTE KI align 16 for UBO struct
struct DirLightUBO {
    glm::vec3 dir;
    int pad1;

    glm::vec3 diffuse;
    int pad2;

    glm::vec3 specular;
    int pad3;
};

// NOTE KI align 16 for UBO struct
struct PointLightUBO {
    glm::vec3 pos;
    int pad1;

    glm::vec3 diffuse;
    int pad2;

    glm::vec3 specular;
    int pad3;

    float constant;
    float linear;
    float quadratic;
    float radius;
};

// NOTE KI align 16 for UBO struct
struct SpotLightUBO {
    glm::vec3 pos;
    int pad1;

    glm::vec3 dir;
    int pad2;

    glm::vec3 diffuse;
    int pad3;

    glm::vec3 specular;
    int pad4;

    float constant;
    float linear;
    float quadratic;

    float cutoff;
    float outerCutoff;
    float radius;

    int pad5;
    int pad6;
};

// NOTE KI align 16 for UBO struct
struct LightsUBO {
    unsigned int dirCount;
    unsigned int pointCount;
    unsigned int spotCount;
    int pad1;

    DirLightUBO dir[1];
    // NOTE KI align 16 for UBO array entries
    PointLightUBO pointLights[MAX_LIGHT_COUNT];
    // NOTE KI align 16 for UBO array entries
    SpotLightUBO spotLights[MAX_LIGHT_COUNT];
};
