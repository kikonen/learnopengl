#pragma once

struct GLBuffer {
    GLBuffer() {
    }

    ~GLBuffer() {
        if (id != -1) {
            glDeleteBuffers(1, &id);
        }
    }

    void create() {
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

    GLuint id = -1;
};
