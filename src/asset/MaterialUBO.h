#pragma once

#include <glm/glm.hpp>

#include "ki/GL.h"

//constexpr unsigned int MIN_MATERIAL_COUNT = 200;
//constexpr unsigned int MAX_MATERIAL_COUNT = 200;
//constexpr unsigned int MATERIAL_COUNT = MAX_MATERIAL_COUNT;


// NOTE KI align 16 for UBO struct
struct MaterialUBO {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 emission;
    glm::vec4 specular;
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
    float tiling;

    //int pad1;
};

// NOTE KI align 16 for UBO struct
struct MaterialsUBO {
    // NOTE KI align 16 for UBO array entries
    MaterialUBO materials[];
};
