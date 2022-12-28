#pragma once

#include "ki/GL.h"

struct GLBlendMode
{
    GLenum srcRGB = GL_SRC_ALPHA;
    GLenum dstRGB = GL_ONE_MINUS_SRC_ALPHA;
    GLenum srcAlpha = GL_ZERO;
    GLenum dstAlpha = GL_ONE;

    bool operator==(const GLBlendMode& o) {
        return srcRGB == o.srcRGB &&
            dstRGB == o.dstRGB &&
            srcAlpha == o.srcAlpha &&
            dstAlpha == o.dstAlpha;
    }

    bool operator!=(const GLBlendMode& o) {
        return !operator==(o);
    }
};
