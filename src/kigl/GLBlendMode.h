#pragma once

#include "kigl/kigl.h"

namespace kigl {
    // NOTE KI defaults based into specified default values in
    // - https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBlendEquationSeparate.xhtml
    // - https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBlendFuncSeparate.xhtml
    struct GLBlendMode
    {
        GLenum m_blendEquationRGB{ GL_FUNC_ADD };
        GLenum m_blendEquationAlpha{ GL_FUNC_ADD };

        GLenum m_srcRGB{ GL_ONE };
        GLenum m_dstRGB{ GL_ZERO };
        GLenum m_srcAlpha{ GL_ONE };
        GLenum m_dstAlpha{ GL_ZERO };

        // Reset to default
        GLBlendMode()
            : GLBlendMode{GL_FUNC_ADD, GL_ONE, GL_ZERO}
        {
        }

        GLBlendMode(
            GLenum src,
            GLenum dst)
            : GLBlendMode{ GL_FUNC_ADD, src, dst, src, dst }
        {
        }

        GLBlendMode(
            GLenum blendEquation,
            GLenum src,
            GLenum dst)
            : GLBlendMode{blendEquation, src, dst, src, dst}
        {
        }

        GLBlendMode(
            GLenum blendEquation,
            GLenum srcRGB,
            GLenum dstRGB,
            GLenum srcAlpha,
            GLenum dstAlpha)
            : GLBlendMode{blendEquation, blendEquation, srcRGB, dstRGB, srcAlpha, dstAlpha}
        {
        }

        GLBlendMode(
            GLenum blendEquationRGB,
            GLenum blendEquationAlpha,
            GLenum srcRGB,
            GLenum dstRGB,
            GLenum srcAlpha,
            GLenum dstAlpha)
            : m_blendEquationRGB{ blendEquationRGB },
            m_blendEquationAlpha{ blendEquationAlpha },
            m_srcRGB{ srcRGB },
            m_dstRGB{ dstRGB },
            m_srcAlpha{ srcAlpha },
            m_dstAlpha{ dstAlpha }
        {
        }

        bool operator==(const GLBlendMode& o)
        {
            return m_blendEquationRGB == o.m_blendEquationRGB &&
                m_blendEquationAlpha == o.m_blendEquationAlpha &&
                m_srcRGB == o.m_srcRGB &&
                m_dstRGB == o.m_dstRGB &&
                m_srcAlpha == o.m_srcAlpha &&
                m_dstAlpha == o.m_dstAlpha;
        }

        bool operator!=(const GLBlendMode& o)
        {
            return !operator==(o);
        }

        void invalidate() {
            *this = { 0, 0, 0, 0, 0 };
        }

        void clear() {
            *this = GLBlendMode{};
        }

        void apply() const
        {
            glBlendEquationSeparate(m_blendEquationRGB, m_blendEquationAlpha);
            glBlendFuncSeparate(m_srcRGB, m_dstRGB, m_srcAlpha, m_dstAlpha);
        }

        void apply(GLuint drawBuffer) const
        {
            glBlendEquationSeparatei(drawBuffer, m_blendEquationRGB, m_blendEquationAlpha);
            glBlendFuncSeparatei(drawBuffer, m_srcRGB, m_dstRGB, m_srcAlpha, m_dstAlpha);
        }
    };
}
