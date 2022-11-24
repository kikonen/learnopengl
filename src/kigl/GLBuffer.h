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

    void initEmpty(int size, int flags) {
        glNamedBufferStorage(id, size, nullptr, flags);
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

    GLuint id = -1;
};
