#pragma once

#include "ki/GL.h"

struct GLBuffer {
    GLBuffer() {
    }

    ~GLBuffer() {
        if (m_mapped) {
            glUnmapNamedBuffer(m_id);
        }
        if (m_created) {
            glDeleteBuffers(1, &m_id);
        }
    }

    operator int() const { return m_id; }

    void create() {
        if (m_created) return;
        glCreateBuffers(1, &m_id);
        m_created = true;
    }

    void createEmpty(int size, int flags) {
        create();
        initEmpty(size, flags);
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

    void bind(GLuint ubo) {
        bindRange(ubo, 0, m_size);
    }

    void bindRange(GLuint ubo, int offset, int size) {
        if (!m_created) return;
        glBindBufferRange(GL_UNIFORM_BUFFER, ubo, m_id, offset, size);
    }

    void bindSSBO(GLuint ssbo) {
        bindSSBORange(ssbo, 0, m_size);
    }

    void bindSSBORange(GLuint ssbo, int offset, int size) {
        if (!m_created) return;
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, ssbo, m_id, offset, size);
    }

    void bindDrawIndirect() {
        if (!m_created) return;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_id);
    }

    // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
    void* map(int flags) {
        mapRange(0, m_size, flags);
    }

    void* mapRange(int offset, int length, int flags) {
        if (!m_created || m_mapped) return m_data;
        m_data = glMapNamedBufferRange(m_id, offset, length, flags);
        m_mapped = true;
        return m_data;
    }

    void flushRange(int offset, int length) {
        if (!m_mapped) return;
        glFlushMappedNamedBufferRange(m_id, offset, length);
    }

    void unmap() {
        glUnmapNamedBuffer(m_id);
        m_data = nullptr;
        m_mapped = false;
    }

    GLuint m_id = 0;
    int m_size = 0;

    void* m_data{ nullptr };

    bool m_created = false;
    bool m_mapped = false;
};
