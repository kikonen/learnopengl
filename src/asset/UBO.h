#pragma once

#include "kigl/kigl.h"

// https://stackoverflow.com/questions/49798189/glbuffersubdata-offsets-for-structs


constexpr GLuint UBO_MATRICES = 0;
constexpr GLuint UBO_DATA = 1;
constexpr GLuint UBO_CLIP_PLANES = 2;
constexpr GLuint UBO_LIGHTS = 3;
//constexpr GLuint UBO_MATERIALS = 4;
constexpr GLuint UBO_TEXTURES = 5;
constexpr GLuint UBO_BUFFER_INFO = 6;
constexpr GLuint UBO_DEBUG = 7;
