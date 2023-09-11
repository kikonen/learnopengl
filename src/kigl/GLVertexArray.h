#pragma once

#include "ki/GL.h"

struct GLVertexArray {
    GLVertexArray() {
    }

    ~GLVertexArray() {
        if (m_created) {
            glDeleteVertexArrays(1, &m_id);
        }
    }

    operator int() const { return m_id; }

    void create(const std::string& name) {
        if (m_created) return;
        glCreateVertexArrays(1, &m_id);
        m_created = true;
        glObjectLabel(GL_VERTEX_ARRAY, m_id, name.length(), name.c_str());
    }

    bool m_created{ false };
    GLuint m_id{ 0 };
};
