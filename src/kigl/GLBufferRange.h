#pragma once

#include "ki/GL.h"

struct GLBufferRange {
    size_t m_maxCount = 0;
    size_t m_count = 0;

    size_t m_index = 0;
    size_t m_offset = 0;
    size_t m_length = 0;

    GLsync m_sync = 0;

    inline size_t next() {
        return m_index + m_count++;
    }

    inline size_t index(size_t idx) {
        return m_index + idx;
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

        size_t count = 0;
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
