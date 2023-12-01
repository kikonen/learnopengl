#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

constexpr size_t MAX_LIGHT_COUNT = 128;

// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o

#pragma pack(push, 1)

// NOTE KI align 16 for UBO struct
struct DirLightUBO {
    glm::vec3 u_dir;
    int pad1;

    // color + a = intensity
    glm::vec4 u_diffuse;
};

// NOTE KI align 16 for UBO struct
struct PointLightUBO {
    glm::vec3 u_pos;
    int pad1;

    // color + a = intensity
    glm::vec4 u_diffuse;

    float u_constant;
    float u_linear;
    float u_quadratic;
    float u_radius;
};

// NOTE KI align 16 for UBO struct
struct SpotLightUBO {
    glm::vec3 u_pos;
    int pad1;

    glm::vec3 u_dir;
    int pad2;

    // color + a = intensity
    glm::vec4 u_diffuse;

    float u_constant;
    float u_linear;
    float u_quadratic;

    float u_cutoff;
    float u_outerCutoff;
    float u_radius;

    int pad5;
    int pad6;
};

// NOTE KI align 16 for UBO struct
struct LightsUBO {
    GLuint u_dirCount;
    GLuint u_pointCount;
    GLuint u_spotCount;
    int pad1;

    DirLightUBO u_dir[1];
    // NOTE KI align 16 for UBO array entries
    PointLightUBO u_pointLights[MAX_LIGHT_COUNT];
    // NOTE KI align 16 for UBO array entries
    SpotLightUBO u_spotLights[MAX_LIGHT_COUNT];
};

#pragma pack(pop)
