#pragma once

#include "ki/GL.h"

struct GLBufferRange {
    int m_maxCount = 0;
    int m_count = 0;

    int m_baseIndex = 0;
    int m_baseOffset = 0;
    GLsync m_sync = 0;

    inline int next() {
        return m_baseIndex + m_count++;
    }

    inline int index(int idx) {
        return m_baseIndex + idx;
    }

    inline bool isFull() {
        return m_count == m_maxCount;
    }

    void clear() {
        m_count = 0;
    }

    void setFence()
    {
        if (m_sync) {
            glDeleteSync(m_sync);
        }
        m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
    void waitFence()
    {
        if (!m_sync) return;

        int count = 0;
        GLenum res = GL_UNSIGNALED;
        while (res != GL_ALREADY_SIGNALED && res != GL_CONDITION_SATISFIED)
        {
            res = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 100000);
            count++;
        }

        glDeleteSync(m_sync);
        m_sync = 0;
    }
};
