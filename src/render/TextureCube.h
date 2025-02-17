#pragma once

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

namespace render {
    class TextureCube {
    public:
        static const TextureCube& get();

        TextureCube()
            : TextureCube(1.f)
        { }

        TextureCube(float scale)
            : m_scale{ scale }
        { }

        ~TextureCube() = default;

        void prepare();

        void draw() const;

        void drawInstanced(int instanceCount);

    private:
        const float m_scale;
        kigl::GLVertexArray m_vao;
        kigl::GLBuffer m_vbo{ "texture_cube" };
    };
}
