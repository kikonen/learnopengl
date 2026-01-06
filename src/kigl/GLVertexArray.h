#pragma once

#include "kigl/GLState.h"

#include "kigl/kigl.h"

namespace kigl {
    struct GLVertexArray {
        GLVertexArray() {
        }

        GLVertexArray(GLVertexArray& o) = delete;

        GLVertexArray(GLVertexArray&& o) noexcept
        {
            swap(o);
        }

        ~GLVertexArray() {
            if (m_created) {
                glDeleteVertexArrays(1, &m_id);
                kigl::GLState::get().invalidateVAO();
            }
        }

        GLVertexArray& operator=(GLVertexArray& o) = delete;
        GLVertexArray& operator=(GLVertexArray&& o) noexcept
        {
            GLVertexArray tmp(std::move(o));
            swap(tmp);
            return *this;
        }

        operator int() const { return m_id; }

        void swap(GLVertexArray& o) noexcept;

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
