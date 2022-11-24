#pragma once

#include "ki/GL.h"

struct GLVertexArray {
    GLVertexArray() {
    }

    ~GLVertexArray() {
        if (id != -1) {
            glDeleteVertexArrays(1, &id);
        }
    }

    operator int() const { return id; }

    void create() {
        if (id != -1) return;
        glCreateVertexArrays(1, &id);
    }

    GLuint id = -1;
};
