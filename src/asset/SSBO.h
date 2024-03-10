#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"

constexpr GLuint SSBO_MATERIALS = 1;
//constexpr GLuint SSBO_TEXTURES = 2;
constexpr GLuint SSBO_ENTITIES = 3;
constexpr GLuint SSBO_MATERIAL_INDECES = 4;
constexpr GLuint SSBO_DRAW_COMMANDS = 5;
constexpr GLuint SSBO_DRAW_PARAMETERS = 6;
constexpr GLuint SSBO_PERFORMANCE_COUNTERS = 7;
constexpr GLuint SSBO_SHAPES = 8;
constexpr GLuint SSBO_INSTANCE_INDECES = 9;
constexpr GLuint SSBO_PARTICLES = 10;
constexpr GLuint SSBO_TERRAIN_TILES = 11;

// NOTE KI align 16 for UBO struct
// OpenGL Superbible, 7th Edition, page 552
// https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
// https://www.khronos.org/opengl/wiki/Bindless_Texture
//struct TextureSSBO {
//    GLuint64 handle;
//};
