#include "TextureCube.h"

#include <glm/glm.hpp>

#include "kigl/GLState.h"

#include "shader/Shader.h"

namespace {
#pragma pack(push, 1)
    struct VertexEntry {
        //kigl::VEC10 pos;
        glm::vec3 u_pos;

        //kigl::VEC10 normal;
        glm::vec3 u_normal;

        //kigl::UV16 u_texCoord;
        glm::vec2 u_texCoord;

        VertexEntry(glm::vec3 p, glm::vec3 n, glm::vec2 t)
            : u_pos{ p },
            u_normal{ n },
            u_texCoord{ t }
        {}
    };
#pragma pack(pop)
}

namespace render {
    void TextureCube::prepare()
    {
        const VertexEntry vertices[] = {
            // back
            VertexEntry{ {-1.0f, -1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f} },
            VertexEntry{ { 1.0f,  1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f} },
            VertexEntry{ { 1.0f, -1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f} },
            VertexEntry{ { 1.0f,  1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f} },
            VertexEntry{ {-1.0f, -1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f} },
            VertexEntry{ {-1.0f,  1.0f, -1.0f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f} },
            // front
            VertexEntry{ {-1.0f, -1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f} },
            VertexEntry{ { 1.0f, -1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f} },
            VertexEntry{ { 1.0f,  1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f} },
            VertexEntry{ { 1.0f,  1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f} },
            VertexEntry{ {-1.0f,  1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f} },
            VertexEntry{ {-1.0f, -1.0f,  1.0f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f} },
            // left
            VertexEntry{ {-1.0f,  1.0f,  1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
            VertexEntry{ {-1.0f,  1.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
            VertexEntry{ {-1.0f, -1.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
            VertexEntry{ {-1.0f, -1.0f, -1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
            VertexEntry{ {-1.0f, -1.0f,  1.0f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },
            VertexEntry{ {-1.0f,  1.0f,  1.0f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
            // right
            VertexEntry{ { 1.0f,  1.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
            VertexEntry{ { 1.0f, -1.0f, -1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
            VertexEntry{ { 1.0f,  1.0f, -1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f} },
            VertexEntry{ { 1.0f, -1.0f, -1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f} },
            VertexEntry{ { 1.0f,  1.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f} },
            VertexEntry{ { 1.0f, -1.0f,  1.0f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f} },
            // bottom
            VertexEntry{ {-1.0f, -1.0f, -1.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f} },
            VertexEntry{ { 1.0f, -1.0f, -1.0f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f} },
            VertexEntry{ { 1.0f, -1.0f,  1.0f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f} },
            VertexEntry{ { 1.0f, -1.0f,  1.0f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f} },
            VertexEntry{ {-1.0f, -1.0f,  1.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f} },
            VertexEntry{ {-1.0f, -1.0f, -1.0f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f} },
            // top
            VertexEntry{ {-1.0f,  1.0f, -1.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f} },
            VertexEntry{ { 1.0f,  1.0f , 1.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f} },
            VertexEntry{ { 1.0f,  1.0f, -1.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f} },
            VertexEntry{ { 1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f} },
            VertexEntry{ {-1.0f,  1.0f, -1.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f} },
            VertexEntry{ {-1.0f,  1.0f,  1.0f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f} },
        };

        m_vao.create("texture_cube");

        m_vbo.create();
        m_vbo.init(sizeof(vertices), vertices, 0);

        glVertexArrayVertexBuffer(m_vao, VBO_VERTEX_BINDING, m_vbo, 0, sizeof(VertexEntry));

        glEnableVertexArrayAttrib(m_vao, ATTR_POS);
        glEnableVertexArrayAttrib(m_vao, ATTR_NORMAL);
        glEnableVertexArrayAttrib(m_vao, ATTR_TEX);

        //glVertexArrayAttribFormat(m_vao, ATTR_POS, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexEntry, u_pos));
        glVertexArrayAttribFormat(m_vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, u_pos));

        //glVertexArrayAttribFormat(m_vao, ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexEntry, u_normal));
        glVertexArrayAttribFormat(m_vao, ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, u_normal));

        //glVertexArrayAttribFormat(m_vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, u_texCoord));
        glVertexArrayAttribFormat(m_vao, ATTR_TEX, 2, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, u_texCoord));

        glVertexArrayAttribBinding(m_vao, ATTR_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(m_vao, ATTR_NORMAL, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(m_vao, ATTR_TEX, VBO_VERTEX_BINDING);

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(m_vao, VBO_VERTEX_BINDING, 0);
    }

    void TextureCube::draw()
    {
        kigl::GLState::get().bindVAO(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}
