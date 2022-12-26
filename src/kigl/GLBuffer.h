#pragma once

#include "ki/GL.h"

struct GLBuffer {
    GLBuffer() {
    }

    ~GLBuffer() {
        if (m_id != -1) {
            glDeleteBuffers(1, &m_id);
        }
    }

    operator int() const { return m_id; }

    void create() {
        if (m_id != -1) return;
        glCreateBuffers(1, &m_id);
    }

    // For mapped buffer
    // flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    void initEmpty(int size, int flags) {
        m_size = size;
        glNamedBufferStorage(m_id, size, nullptr, flags);
    }

    void init(int size, void* data, int flags) {
        m_size = size;
        glNamedBufferStorage(m_id, size, data, flags);
    }

    void update(int offset, int length, void* data) {
        glNamedBufferSubData(m_id, offset, length, data);
    }

    void bindUniform(GLuint ubo) {
        if (m_id == -1) return;
        glBindBufferBase(GL_UNIFORM_BUFFER, ubo, m_id);
    }

    void bindSSBO(GLuint ssbo) {
        if (m_id == -1) return;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo, m_id);
    }

    void bindDrawIndirect() {
        if (m_id == -1) return;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_id);
    }

    // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
    void* map(int flags) {
        return glMapNamedBuffer(m_id, flags);
    }

    void* mapRange(int offset, int length, int flags) {
        return glMapNamedBufferRange(m_id, offset, length, flags);
    }

    void flushRange(int offset, int length) {
        glFlushMappedNamedBufferRange(m_id, offset, length);
    }

    void unmap() {
        glUnmapNamedBuffer(m_id);
    }

    GLuint m_id = -1;

    int m_size;
};
