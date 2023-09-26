#pragma once

#include "ki/GL.h"

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

    GLFrameBufferHandle(GLFrameBufferHandle& handle) = delete;
    GLFrameBufferHandle& operator=(GLFrameBufferHandle& handle) = delete;

    GLFrameBufferHandle(GLFrameBufferHandle&& handle) noexcept
        : m_fbo(handle.m_fbo)
    {
        handle.m_fbo = 0;
    }

    GLFrameBufferHandle& operator=(GLFrameBufferHandle&& handle) noexcept
    {
        m_fbo = handle.m_fbo;
        handle.m_fbo = 0;
        return *this;
    }

    void create(std::string_view name) {
        if (m_fbo > 0) return;
        glCreateFramebuffers(1, &m_fbo);

        glObjectLabel(GL_FRAMEBUFFER, m_fbo, name.length(), name.data());

        KI_INFO(fmt::format("CREATE: name={}, FBO={},", name, m_fbo));
    }

    operator int() const { return m_fbo; }

public:
    GLuint m_fbo{ 0 };
};
