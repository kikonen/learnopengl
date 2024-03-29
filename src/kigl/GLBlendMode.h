#pragma once

#include "kigl/kigl.h"

namespace kigl {
    struct GLBlendMode
    {
        GLenum blendEquation = GL_FUNC_ADD;
        GLenum srcRGB = GL_SRC_ALPHA;
        GLenum dstRGB = GL_ONE_MINUS_SRC_ALPHA;
        GLenum srcAlpha = GL_ZERO;
        GLenum dstAlpha = GL_ONE;

        bool operator==(const GLBlendMode& o) {
            return blendEquation == o.blendEquation &&
                srcRGB == o.srcRGB &&
                dstRGB == o.dstRGB &&
                srcAlpha == o.srcAlpha &&
                dstAlpha == o.dstAlpha;
        }

        bool operator!=(const GLBlendMode& o) {
            return !operator==(o);
        }

        void apply() {
            glBlendEquationSeparate(blendEquation, blendEquation);

            // NOTE KI FrameBufferAttachment::getTextureRGB() also fixes this
            //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
        }
    };
}
