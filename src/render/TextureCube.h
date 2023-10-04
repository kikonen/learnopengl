#pragma once

#include "kigl/kigl.h"

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

class GLState;

class TextureCube {
public:
    TextureCube() = default;
    ~TextureCube() = default;

    void prepare();

    void draw(GLState& state);

private:
    GLVertexArray m_vao;
    GLBuffer m_vbo{ "texture_cube" };
};
