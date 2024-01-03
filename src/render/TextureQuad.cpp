#include "TextureQuad.h"

#include "kigl/GLState.h"

#include "asset/Shader.h"

namespace {
#pragma pack(push, 1)
    struct VertexEntry {
        kigl::VEC10 pos;
        kigl::UV16 texCoord;

        VertexEntry(glm::vec3 p, glm::vec2 t)
            : pos{ p },
            texCoord{ t }
        {}
    };
#pragma pack(pop)
}

void TextureQuad::prepare()
{
    // NOTE KI z == 1.0 for skybox
    const VertexEntry vertices[4] = {
        VertexEntry{ {-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f} },
        VertexEntry{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },
        VertexEntry{ { 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f} },
        VertexEntry{ { 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
    };

    m_vao.create("texture_quad");

    m_vbo.create();

    m_vbo.init(sizeof(vertices), vertices, 0);

    {
        glVertexArrayVertexBuffer(m_vao, VBO_VERTEX_BINDING, m_vbo, 0, sizeof(VertexEntry));

        glEnableVertexArrayAttrib(m_vao, ATTR_POS);
        glEnableVertexArrayAttrib(m_vao, ATTR_TEX);

        glVertexArrayAttribFormat(m_vao, ATTR_POS, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexEntry, pos));
        glVertexArrayAttribFormat(m_vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, texCoord));

        glVertexArrayAttribBinding(m_vao, ATTR_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(m_vao, ATTR_TEX, VBO_VERTEX_BINDING);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(m_vao, VBO_VERTEX_BINDING, 0);
    }
}

void TextureQuad::draw(kigl::GLState& state)
{
    state.bindVAO(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //glBindVertexArray(0);
}
