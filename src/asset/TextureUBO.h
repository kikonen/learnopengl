#pragma once

#include <glm/glm.hpp>

#include "ki/GL.h"

// MAX textures used in shader
constexpr unsigned int MIN_TEXTURE_COUNT = 256;
constexpr unsigned int MAX_TEXTURE_COUNT = 256;
constexpr unsigned int TEXTURE_COUNT = MAX_TEXTURE_COUNT;

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
