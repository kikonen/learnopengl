#pragma once

#include "kigl/kigl.h"

inline constexpr uint8_t MIP_MAP_LEVELS = 12;

struct TextureSpec {
    // NOTE KI opengl default is GL_REPEAT
    // => match that
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
    uint16_t wrapS = GL_REPEAT;
    uint16_t wrapT = GL_REPEAT;

    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
    // https://community.khronos.org/t/gl-nearest-mipmap-linear-or-gl-linear-mipmap-nearest/37648/5
    //
    // NOTE KI GL_NEAREST_MIPMAP_LINEAR is *default*
    uint16_t minFilter = GL_LINEAR_MIPMAP_NEAREST;
    uint16_t magFilter = GL_LINEAR;

    // Max mipmap levels
    uint8_t mipMapLevels = MIP_MAP_LEVELS;
};
