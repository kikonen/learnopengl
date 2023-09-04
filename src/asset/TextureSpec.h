#pragma once

#include "ki/GL.h"

constexpr int MIP_MAP_LEVELS = 5;

struct TextureSpec {
    // NOTE KI opengl default is GL_REPEAT
    // => match that
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
    GLint wrapS = GL_REPEAT;
    GLint wrapT = GL_REPEAT;

    int minFilter = GL_NEAREST_MIPMAP_LINEAR;
    int magFilter = GL_LINEAR;

    int mipMapLevels = MIP_MAP_LEVELS;
};
