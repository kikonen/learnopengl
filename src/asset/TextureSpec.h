#pragma once

#include "ki/GL.h"

constexpr int MIP_MAP_LEVELS = 5;

struct TextureSpec {
    GLint wrapS = GL_CLAMP_TO_EDGE;
    GLint wrapT = GL_CLAMP_TO_EDGE;

    int minFilter = GL_NEAREST_MIPMAP_LINEAR;
    int magFilter = GL_LINEAR;

    int mipMapLevels = MIP_MAP_LEVELS;
};
