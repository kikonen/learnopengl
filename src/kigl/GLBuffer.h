#pragma once

#include <fmt/format.h>

#include "ki/GL.h"

struct GLBuffer {
    GLBuffer(const std::string name)
        : m_name(name)
    {
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
        KI_DEBUG(fmt::format("BUFFER: create - name={}, id={}", m_name, m_id));
        m_created = true;
    }

    void createEmpty(int size, int flags) {
        create();
        initEmpty(size, flags);
    }

    void resizeBuffer(int size)
    {
        if (size == m_size) return;
        unmap();
        if (m_created) {
            glDeleteBuffers(1, &m_id);
            m_created = false;
        }
        createEmpty(size, m_flags);
    }

    // For mapped buffer
    // flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    void initEmpty(int size, int flags) {
        m_size = size;
        m_flags = flags;

        KI_DEBUG(fmt::format(
            "BUFFER: initEmpty - name={}, id={}, size={}, flags={}",
            m_name, m_id, m_size, flags));

        glNamedBufferStorage(m_id, size, nullptr, flags);
    }

    void init(int size, void* data, int flags) {
        m_size = size;

        KI_DEBUG(fmt::format(
            "BUFFER: init - name={}, id={}, size={}, flags={}",
            m_name, m_id, m_size, flags));

        glNamedBufferStorage(m_id, size, data, flags);
    }

    void clear() {
        glClearNamedBufferSubData(
            m_id,
            GL_R32UI,
            0,
            m_size,
            GL_RED_INTEGER,
            GL_UNSIGNED_INT,
            nullptr);
    }

    void update(size_t offset, size_t length, void* data) {
        glNamedBufferSubData(m_id, offset, length, data);
    }

    void get(void* data) {
        glGetNamedBufferSubData(m_id, 0, m_size, data);
    }

    void getRange(size_t offset, size_t length, void* data) {
        glGetNamedBufferSubData(m_id, offset, length, data);
    }

    void bind(GLuint ubo) {
        bindRange(ubo, 0, m_size);
    }

    void bindRange(GLuint ubo, size_t offset, size_t length) {
        bindBufferRange(GL_UNIFORM_BUFFER, ubo, offset, length);
    }

    void bindSSBO(GLuint ssbo) {
        bindSSBORange(ssbo, 0, m_size);
    }

    void bindSSBORange(GLuint ssbo, size_t offset, size_t length) {
        bindBufferRange(GL_SHADER_STORAGE_BUFFER, ssbo, offset, length);
    }

    void bindAtomicCounter(GLuint atomic) {
        bindBufferRange(GL_ATOMIC_COUNTER_BUFFER, atomic, 0, m_size);
    }

    void bindBufferRange(GLenum target, GLuint binding, size_t offset, size_t length) {
        if (!m_created) return;

        if (m_binding != binding) {
            m_binding = binding;

            KI_DEBUG(fmt::format(
                "BUFFER: bindBufferRange - name={}, id={}, size={}, binding={}, offset={}, len={}",
                m_name, m_id, m_size, m_binding, offset, length));
        }

        glBindBufferRange(target, binding, m_id, offset, length);
    }

    void bindParameter() {
        if (!m_created) return;
        glBindBuffer(GL_PARAMETER_BUFFER, m_id);
    }

    void bindDrawIndirect() {
        if (!m_created) return;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_id);
    }

    void bindVBO(int vao, int binding, int strideSize) {
        glVertexArrayVertexBuffer(vao, binding, m_id, 0, strideSize);
    }

    // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMapBufferRange.xhtml
    unsigned char* map(int flags) {
        //return (unsigned char*)glMapNamedBuffer(m_id, flags);
        return mapRange(0, m_size, flags);
    }

    unsigned char* mapRange(size_t offset, size_t length, int flags) {
        if (!m_created || m_mapped) return m_data;

        KI_DEBUG(fmt::format(
            "BUFFER: mapRange - name={}, id={}, size={}, offset={}, len={}, flags={}",
            m_name, m_id, m_size, offset, length, flags));

        m_data = (unsigned char*)glMapNamedBufferRange(m_id, offset, length, flags);
        m_mapped = true;
        return m_data;
    }

    void flushRange(size_t offset, size_t length) {
        if (!m_mapped) return;
        glFlushMappedNamedBufferRange(m_id, offset, length);
    }

    void unmap() {
        if (!m_mapped) return;
        glUnmapNamedBuffer(m_id);
        m_data = nullptr;
        m_mapped = false;
    }

    const std::string m_name;

    GLuint m_id = 0;
    size_t m_size = 0;
    int m_flags = 0;

    int m_binding = -1;

    unsigned char* m_data{ nullptr };

    bool m_created = false;
    bool m_mapped = false;
};
