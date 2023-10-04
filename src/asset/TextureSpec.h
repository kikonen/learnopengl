#pragma once

#include "kigl/kigl.h"

constexpr int MIP_MAP_LEVELS = 5;

struct TextureSpec {
    // NOTE KI opengl default is GL_REPEAT
    // => match that
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
    GLint wrapS = GL_REPEAT;
    GLint wrapT = GL_REPEAT;

    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
    // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
    // 
    // NOTE KI GL_NEAREST_MIPMAP_LINEAR is *default*
    int minFilter = GL_LINEAR_MIPMAP_NEAREST;
    int magFilter = GL_LINEAR;

    int mipMapLevels = MIP_MAP_LEVELS;
};
