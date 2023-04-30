#pragma once

#include "ki/GL.h"

struct GLBufferRange {
    bool m_debug{ false };

    size_t m_maxCount = 0;
    size_t m_usedCount = 0;

    size_t m_index = 0;
    size_t m_entrySize = 0;
    size_t m_baseIndex = 0;
    size_t m_baseOffset = 0;
    size_t m_length = 0;
    size_t m_paddedLength = 0;

    GLsync m_sync = 0;

    inline size_t nextOffset() {
        return m_baseOffset + (m_entrySize * m_usedCount++);
    }

    inline size_t offset(size_t idx) const {
        return m_baseIndex + (m_entrySize * idx);
    }

    inline bool full() const {
        return m_usedCount == m_maxCount;
    }

    inline bool empty() const {
        return m_usedCount == 0;
    }

    inline size_t getUsedLength() const {
        return m_entrySize * m_usedCount;
    }

    inline size_t getLengthFor(int count) const {
        return m_entrySize * count;
    }

    void clear() {
        m_usedCount = 0;
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
            res = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 10000);
            count++;
        }

        if (m_debug) {
            if (count > 1) {
                std::cout << '[' << count << ']';
            }
            //else {
            //    std::cout << '.';
            //}
        }

        glDeleteSync(m_sync);
        m_sync = 0;
    }

    void waitFenceOnServer()
    {
        if (m_sync) {
            // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glWaitSync.xhtml
            glWaitSync(m_sync, 0, GL_TIMEOUT_IGNORED);
        }
    }
};
