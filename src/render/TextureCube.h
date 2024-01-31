#pragma once

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

namespace render {
    class TextureCube {
    public:
        TextureCube() = default;
        ~TextureCube() = default;

        void prepare();

        void draw();

    private:
        kigl::GLVertexArray m_vao;
        kigl::GLBuffer m_vbo{ "texture_cube" };
    };
}
