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
    glm::vec4 ambient; // 16
    glm::vec4 diffuse; // 16
    glm::vec4 emission; // 16
    glm::vec4 specular; // 16
    float shininess;

    // NOTE KI "tex index", not "unit index"
    int diffuseTex;
    int emissionTex;
    int specularTex;
    int normalMap;
    int dudvMap;

    int pattern;

    float reflection;
    float refraction;

    float refractionRatio;

    float fogRatio;

    float tilingX;
    float tilingY;

    int pad1;
    int pad2;
    int pad3;
};
