#include "TextureQuad.h"

#include "kigl/GLState.h"

#include "asset/Shader.h"

namespace {
#pragma pack(push, 1)
    struct VertexEntry {
        glm::vec3 pos;
        ki::UV16 texCoords;

        VertexEntry(glm::vec3 p, glm::vec2 t) {
            pos = p;
            texCoords.u = (int)(t.x * ki::SCALE_UV16);
            texCoords.v = (int)(t.y * ki::SCALE_UV16);
        }
    };
#pragma pack(pop)
}

void TextureQuad::prepare()
{
    // NOTE KI z == 1.0 for skybox
    const VertexEntry vertices[4] = {
        VertexEntry{ {-1.0f,  1.0f, 1.0f}, {0.0f, 1.0f} },
        VertexEntry{ {-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f} },
        VertexEntry{ { 1.0f,  1.0f, 1.0f}, {1.0f, 1.0f} },
        VertexEntry{ { 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f} },
    };

    m_vao.create("texture_quad");

    m_vbo.create();

    m_vbo.init(sizeof(vertices), vertices, 0);

    {
        glVertexArrayVertexBuffer(m_vao, VBO_VERTEX_BINDING, m_vbo, 0, sizeof(VertexEntry));

        glEnableVertexArrayAttrib(m_vao, ATTR_POS);
        glEnableVertexArrayAttrib(m_vao, ATTR_TEX);

        glVertexArrayAttribFormat(m_vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, pos));
        glVertexArrayAttribFormat(m_vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, texCoords));

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
