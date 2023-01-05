#pragma once

#include <glm/glm.hpp>

#include "ki/GL.h"

#include "kigl/GLBuffer.h"

// https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs


struct UBO {
    GLBuffer matrices;
    GLBuffer data;
    GLBuffer clipPlanes;
    GLBuffer lights;
};

constexpr GLuint UBO_MATRICES = 0;
constexpr GLuint UBO_DATA = 1;
constexpr GLuint UBO_CLIP_PLANES = 2;
constexpr GLuint UBO_LIGHTS = 3;
constexpr GLuint UBO_MATERIALS = 4;
constexpr GLuint UBO_TEXTURES = 5;

constexpr unsigned int MIN_MATERIAL_COUNT = 200;
constexpr unsigned int MAX_MATERIAL_COUNT = 200;
constexpr unsigned int MATERIAL_COUNT = MAX_MATERIAL_COUNT;

constexpr unsigned int MIN_LIGHT_COUNT = 8;
constexpr unsigned int MAX_LIGHT_COUNT = 8;
constexpr unsigned int LIGHT_COUNT = MAX_LIGHT_COUNT;

// MAX textures used in shader
constexpr unsigned int MIN_TEXTURE_COUNT = 256;
constexpr unsigned int MAX_TEXTURE_COUNT = 256;
constexpr unsigned int TEXTURE_COUNT = MAX_TEXTURE_COUNT;

constexpr unsigned int MIN_CLIP_PLANE_COUNT = 2;
constexpr unsigned int MAX_CLIP_PLANE_COUNT = 2;
constexpr unsigned int CLIP_PLANE_COUNT = MAX_CLIP_PLANE_COUNT;

constexpr unsigned int TEXTURE_UNIT_COUNT = 64;
constexpr unsigned int FIRST_TEXTURE_UNIT = 0;
constexpr unsigned int LAST_TEXTURE_UNIT = FIRST_TEXTURE_UNIT + TEXTURE_UNIT_COUNT - 1;

#define ASSERT_TEX_INDEX(texIndex) assert(texIndex >= 0 && texIndex < TEXTURE_COUNT)

#define ASSERT_TEX_UNIT(unitIndex) assert(unitIndex >= FIRST_TEXTURE_UNIT && unitIndex <= LAST_TEXTURE_UNIT)


// NOTE KI align 16 for UBO struct
struct MatricesUBO {
    glm::mat4 projected;
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 lightProjected;
    glm::mat4 shadow;
};

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

// NOTE KI align 16 for UBO struct
struct DirLightUBO {
    glm::vec3 pos;
    int pad1;
    glm::vec3 dir;
    int pad2;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
};

// NOTE KI align 16 for UBO struct
struct PointLightUBO {
    glm::vec3 pos;
    int pad1;

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

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
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;

    float constant;
    float linear;
    float quadratic;

    float cutoff;
    float outerCutoff;
    float radius;

    int pad3;
    int pad4;
};

// NOTE KI align 16 for UBO struct
struct LightsUBO {
    unsigned int dirCount;
    unsigned int pointCount;
    unsigned int spotCount;
    int pad1;

    DirLightUBO dir[1];
    // NOTE KI align 16 for UBO array entries
    PointLightUBO pointLights[LIGHT_COUNT];
    // NOTE KI align 16 for UBO array entries
    SpotLightUBO spotLights[LIGHT_COUNT];
};

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
    MaterialUBO materials[MATERIAL_COUNT];
};

// NOTE KI align 16 for UBO struct
struct ClipPlaneUBO {
    glm::vec4 plane;
    bool enabled;

    int pad1;
    int pad2;
    int pad3;
};

// NOTE KI align 16 for UBO struct
struct ClipPlanesUBO {
    // NOTE KI align 16 for UBO array entries
    ClipPlaneUBO clipping[CLIP_PLANE_COUNT];
    int clipCount;

    int pad1;
    int pad2;
    int pad3;
};

// NOTE KI align 16 for UBO struct
// OpenGL Superbible, 7th Edition, page 552
// https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
// https://www.khronos.org/opengl/wiki/Bindless_Texture
struct TextureUBO {
    GLuint64 handle;
    int pad1;
    int pad2;
};

// NOTE KI align 16 for UBO struct
struct TexturesUBO {
    // NOTE KI align 16 for array entries
    TextureUBO textures[TEXTURE_COUNT];
};
