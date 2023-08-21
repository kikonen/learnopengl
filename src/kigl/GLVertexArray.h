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

    void create(const std::string& name) {
        if (id != -1) return;
        glCreateVertexArrays(1, &id);
        glObjectLabel(GL_VERTEX_ARRAY, id, name.length(), name.c_str());
    }

    GLuint id = -1;
};
