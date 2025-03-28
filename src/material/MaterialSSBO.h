#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

// NOTE KI align 16 for UBO struct
//
// NOTE KI SSBO array alignment
// OpenGL Programming Guide, 8th Edition, page 887
//
// "Structure alignment is the same as the
// alignment for the biggest structure
// member, where three - component
// vectors are not rounded up to the size of
// four - component vectors.Each structure
// will start on this alignment, and its size
// will be the space needed by its
// members, according to the previous
// rules, rounded up to a multiple of the
// structure alignment."
//
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
//
#pragma pack(push, 1)
struct MaterialSSBO {
    glm::vec4 u_diffuse; // 16
    glm::vec4 u_emission; // 16

    // MRAO: [metalness, roughness, ambient-occlusion, opacity]
    glm::vec4 u_mrao; // 16

    // NOTE KI "tex index", not "unit index"
    GLuint64 u_diffuseTex;
    GLuint64 u_emissionTex;
    //GLuint64 u_specularTex;
    GLuint64 u_normalMap;

    // MRAO: [metalness, roughness, ambient-occlusion, opacity]
    //GLuint64 u_opacityMap;
    GLuint64 u_mraoMap;
    GLuint64 displacementMapTex;

    GLuint64 u_dudvMap;
    GLuint64 u_noiseMap;
    GLuint64 u_noise2Map;

    GLuint64 u_custom1Map;

    int u_pattern;

    float u_reflection;
    float u_refraction;
    float u_refractionRatio;

    float u_tilingX;
    float u_tilingY;

    GLuint u_spriteCount;
    GLuint u_spritesX;
    GLuint u_spritesY;

    int u_layers;
    float u_layersDepth;
    float u_parallaxDepth;

    int pad3_1;
    int pad3_2;
    //int pad3_3;
};
#pragma pack(pop)
