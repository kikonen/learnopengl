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
// 
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
//
struct MaterialSSBO {
    glm::vec4 diffuse; // 16
    glm::vec4 emission; // 16

    // specular + shininess
    glm::vec4 specular; // 16

    float ambient;

    // NOTE KI "tex index", not "unit index"
    int diffuseTex;
    int emissionTex;
    int specularTex;
    int normalMap;

    int dudvMap;
    int heightMap;
    int noiseMap;
    int pattern;

    float reflection;
    float refraction;
    float refractionRatio;

    float tilingX;
    float tilingY;
    int layers;
    float depth;

    //int pad3_1;
    //int pad3_2;
//    int pad3_3;
};
