#pragma once

#include <glm/glm.hpp>

#include "ki/GL.h"

#include "kigl/GLBuffer.h"

constexpr GLuint SSBO_MATERIALS = 1;
constexpr GLuint SSBO_TEXTURES = 2;
constexpr GLuint SSBO_MESHES = 3;
constexpr GLuint SSBO_MATERIAL_INDECES = 4;

//constexpr unsigned int MIN_MATERIAL_COUNT = 200;
//constexpr unsigned int MAX_MATERIAL_COUNT = 200;
//constexpr unsigned int MATERIAL_COUNT = MAX_MATERIAL_COUNT;
//
//constexpr unsigned int MIN_TEXTURE_COUNT = 256;
//constexpr unsigned int MAX_TEXTURE_COUNT = 256;
//constexpr unsigned int TEXTURE_COUNT = MAX_TEXTURE_COUNT;

constexpr size_t MAX_SSBO_TEXTURES = 1000;
constexpr size_t MAX_SSBO_MATERIALS = 1000;

// NOTE KI align 16 for UBO struct
// OpenGL Superbible, 7th Edition, page 552
// https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
// https://www.khronos.org/opengl/wiki/Bindless_Texture
struct TextureSSBO {
    GLuint64 handle;
};

// NOTE KI align 16 for UBO struct
struct MaterialSSBO {
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
};
