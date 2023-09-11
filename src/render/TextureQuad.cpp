#include "TextureQuad.h"

#include "kigl/GLState.h"

#include "asset/Shader.h"


void TextureQuad::prepare()
{
    constexpr float vertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    m_vao.create("texture_quad");

    constexpr size_t VERTEX_ENTRY_SIZE = 5;

    m_vbo.create();
    m_vbo.init(sizeof(vertices), vertices, 0);

    {
        glVertexArrayVertexBuffer(m_vao, VBO_VERTEX_BINDING, m_vbo, 0, sizeof(float) * VERTEX_ENTRY_SIZE);

        glEnableVertexArrayAttrib(m_vao, ATTR_POS);
        glEnableVertexArrayAttrib(m_vao, ATTR_TEX);

        glVertexArrayAttribFormat(m_vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribFormat(m_vao, ATTR_TEX, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

        glVertexArrayAttribBinding(m_vao, ATTR_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(m_vao, ATTR_TEX, VBO_VERTEX_BINDING);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(m_vao, VBO_VERTEX_BINDING, 0);
    }
}

void TextureQuad::draw(GLState& state)
{
    state.bindVAO(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //glBindVertexArray(0);
}
