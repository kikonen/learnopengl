#pragma once

#include "ki/GL.h"

class GLRenderBufferHandle {
public:
    GLRenderBufferHandle()
    {}

    GLRenderBufferHandle(int fbo)
        : m_rbo(fbo)
    {}

    ~GLRenderBufferHandle() {
        if (m_rbo <= 0) return;

        glDeleteRenderbuffers(1, &m_rbo);
    }

    GLRenderBufferHandle(GLRenderBufferHandle& handle) = delete;
    GLRenderBufferHandle& operator=(GLRenderBufferHandle& handle) = delete;

    GLRenderBufferHandle(GLRenderBufferHandle&& handle) noexcept
        : m_rbo(handle.m_rbo)
    {
        handle.m_rbo = 0;
    }

    GLRenderBufferHandle& operator=(GLRenderBufferHandle&& handle)
    {
        m_rbo = handle.m_rbo;
        handle.m_rbo = 0;
        return *this;
    }

    void create(std::string_view name) {
        if (m_rbo > 0) return;
        glCreateRenderbuffers(1, &m_rbo);

        glObjectLabel(GL_RENDERBUFFER, m_rbo, name.length(), name.data());

        KI_INFO(fmt::format("CREATE: name={}, FBO={},", name, m_rbo));
    }

    operator int() const { return m_rbo; }

public:
    GLuint m_rbo{ 0 };
};
