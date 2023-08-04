#pragma once

#include <glm/glm.hpp>

#include "ki/GL.h"

// MAX textures used in shader
constexpr unsigned int MAX_TEXTURE_COUNT = 256;

#pragma pack(push, 1)

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
    TextureUBO textures[MAX_TEXTURE_COUNT];
};

#pragma pack(pop)
