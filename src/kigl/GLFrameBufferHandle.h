#pragma once

#include "kigl/kigl.h"

namespace kigl {
    class GLFrameBufferHandle {
    public:
        GLFrameBufferHandle()
        {}

        GLFrameBufferHandle(int fbo)
            : m_fbo(fbo)
        {}

        ~GLFrameBufferHandle() {
            if (m_fbo <= 0) return;

            glDeleteFramebuffers(1, &m_fbo);
        }

        GLFrameBufferHandle(GLFrameBufferHandle& o) = delete;
        GLFrameBufferHandle& operator=(GLFrameBufferHandle& o) = delete;

        GLFrameBufferHandle(GLFrameBufferHandle&& o) noexcept
        {
            swap(o);
        }

        GLFrameBufferHandle& operator=(GLFrameBufferHandle&& o) noexcept
        {
            GLFrameBufferHandle tmp(std::move(o));
            swap(tmp);
            return *this;
        }

        void swap(GLFrameBufferHandle& o) noexcept;

        void create(std::string_view name) {
            if (m_fbo > 0) return;
            glCreateFramebuffers(1, &m_fbo);
            glNamedFramebufferReadBuffer(m_fbo, GL_NONE);

            kigl::setLabel(GL_FRAMEBUFFER, m_fbo, name);

            KI_INFO(fmt::format("CREATE: name={}, FBO={},", name, m_fbo));
        }

        operator int() const { return m_fbo; }

    public:
        GLuint m_fbo{ 0 };
    };
}
