#pragma once

#include "ki/GL.h"

struct GLBufferRange {
    size_t m_maxCount = 0;
    size_t m_count = 0;

    size_t m_entrySize = 0;
    size_t m_baseIndex = 0;
    size_t m_baseOffset = 0;
    size_t m_length = 0;
    size_t m_paddedLength = 0;

    GLsync m_sync = 0;

    inline size_t nextOffset() {
        return m_baseOffset + (m_entrySize * m_count++);
    }

    inline size_t offset(size_t idx) const {
        return m_baseIndex + (m_entrySize * idx);
    }

    inline bool isFull() const {
        return m_count == m_maxCount;
    }

    inline bool isEmpty() const {
        return m_count == 0;
    }

    inline size_t getUsedSize() const {
        return m_entrySize * m_count;
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
            res = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 500000);
            count++;
        }

        //if (count > 1)
        //    std::cout << '[' << count << ']';
        glDeleteSync(m_sync);
        m_sync = 0;
    }
};
