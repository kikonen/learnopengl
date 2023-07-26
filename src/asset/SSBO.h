#pragma once

#include <glm/glm.hpp>

#include "ki/GL.h"

#include "kigl/GLBuffer.h"

constexpr GLuint SSBO_MATERIALS = 1;
//constexpr GLuint SSBO_TEXTURES = 2;
constexpr GLuint SSBO_ENTITIES = 3;
constexpr GLuint SSBO_MATERIAL_INDECES = 4;
constexpr GLuint SSBO_DRAW_COMMANDS = 5;
constexpr GLuint SSBO_DRAW_PARAMETERS = 6;
constexpr GLuint SSBO_PERFORMANCE_COUNTERS = 7;
constexpr GLuint SSBO_SPRITES = 8;

// NOTE KI align 16 for UBO struct
// OpenGL Superbible, 7th Edition, page 552
// https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
// https://www.khronos.org/opengl/wiki/Bindless_Texture
//struct TextureSSBO {
//    GLuint64 handle;
//};
