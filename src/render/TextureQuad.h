#pragma once

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class GLState;

class TextureQuad {
public:
    TextureQuad() = default;
    ~TextureQuad() = default;

    void prepare();

    void draw(GLState& state);

private:
    GLVertexArray m_vao;
    GLBuffer m_vbo{ "texture_quad" };
};
