#pragma once

#include <glm/glm.hpp>

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
struct MaterialSSBO {
    glm::vec4 diffuse; // 16
    glm::vec3 specular; // 16
    int pad0;
    glm::vec3 emission; // 16
    int pad1;

    float ambient;

    // NOTE KI "tex index", not "unit index"
    int diffuseTex;
    int specularTex;
    int emissionTex;
    int normalMap;

    int dudvMap;
    int heightMap;
    int noiseMap;
    int pattern;

    float shininess;
    float reflection;
    float refraction;
    float refractionRatio;

    float tilingX;
    float tilingY;
    int layers;
    float depth;

    int pad2_1;
    int pad2_2;
    int pad2_3;
};
