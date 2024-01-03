#pragma once

#include "kigl/kigl.h"

namespace kigl {
    struct GLVertexArray {
        GLVertexArray() {
        }

        ~GLVertexArray() {
            if (m_created) {
                glDeleteVertexArrays(1, &m_id);
            }
        }

        operator int() const { return m_id; }

        void create(std::string_view name) {
            if (m_created) return;
            glCreateVertexArrays(1, &m_id);
            m_created = true;
            kigl::setLabel(GL_VERTEX_ARRAY, m_id, name);
        }

        bool m_created{ false };
        GLuint m_id{ 0 };
    };
}
