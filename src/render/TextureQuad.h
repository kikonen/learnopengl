#pragma once

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

namespace kigl {
    class GLState;
}

class TextureQuad {
public:
    TextureQuad() = default;
    ~TextureQuad() = default;

    void prepare();

    void draw(kigl::GLState& state);

private:
    kigl::GLVertexArray m_vao;
    kigl::GLBuffer m_vbo{ "texture_quad" };
};
