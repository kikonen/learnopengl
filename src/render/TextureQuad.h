#pragma once

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"


namespace render {
    class TextureQuad {
    public:
        static TextureQuad& get() noexcept;

        TextureQuad() = default;
        ~TextureQuad() = default;

        void prepare();

        void draw();

        GLuint getBaseIndex() const noexcept {
            return 0;
        }

        GLuint getBaseVertex() const noexcept {
            return 0;
        }

        GLuint getIndexCount() const noexcept {
            return 4;
        }

        const kigl::GLVertexArray& getVao() const noexcept {
            return m_vao;
        }

    private:
        bool m_prepared{ false };
        kigl::GLVertexArray m_vao;
        kigl::GLBuffer m_vbo{ "texture_quad" };
    };
}
