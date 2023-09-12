#pragma once

#include "ki/GL.h"

class GLFrameBufferHandle {
public:
    GLFrameBufferHandle()
    {}

    GLFrameBufferHandle(int fbo)
    : m_fbo(fbo)
    {}

    void create(const std::string& name) {
        if (m_fbo > 0) return;
        glCreateFramebuffers(1, &m_fbo);

        glObjectLabel(GL_FRAMEBUFFER, m_fbo, name.length(), name.c_str());
    }

    ~GLFrameBufferHandle() {
        if (m_fbo > 0) {
            glDeleteFramebuffers(1, &m_fbo);
        }
    }

    operator int() const { return m_fbo; }

public:
    GLuint m_fbo{ 0 };
};
