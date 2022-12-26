#pragma once

#include "ki/GL.h"

struct GLBuffer {
    GLBuffer() {
    }

    ~GLBuffer() {
        if (id != -1) {
            glDeleteBuffers(1, &id);
        }
    }

    operator int() const { return id; }

    void create() {
        if (id != -1) return;
        glCreateBuffers(1, &id);
    }

    // For mapped buffer
    // flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    void initEmpty(int size, int flags) {
        glNamedBufferStorage(id, size, nullptr, flags);
    }

    void init(int size, void* data, int flags) {
        glNamedBufferStorage(id, size, data, flags);
    }

    void update(int offset, int size, void* data) {
        glNamedBufferSubData(id, offset, size, data);
    }

    void bindUniform(GLuint ubo) {
        if (id == -1) return;
        glBindBufferBase(GL_UNIFORM_BUFFER, ubo, id);
    }

    void bindSSBO(GLuint ssbo) {
        if (id == -1) return;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo, id);
    }

    void bindDrawIndirect() {
        if (id == -1) return;
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, id);
    }

    // https://www.cppstories.com/2015/01/persistent-mapped-buffers-in-opengl/
    // flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    void* map(int offset, int size, int flags) {
        return glMapNamedBufferRange(id, offset, size, flags);
    }

    void unmap() {
        glUnmapNamedBuffer(id);
    }

    GLuint id = -1;
};
