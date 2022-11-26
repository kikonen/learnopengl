#include "QuadVBO.h"

#include "glm/glm.hpp"

#include "Shader.h"

namespace {
    const float VERTICES[] = {
        // pos              // normal         // tangent        // tex
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    constexpr int ROW_SIZE = 11;
    constexpr int VERTEX_COUNT = 4;

#pragma pack(push, 1)
    struct TexVBO {
        glm::vec3 pos;
        ki::VEC10 normal;
        ki::VEC10 tangent;
        ki::UV16 texCoords;
    };
#pragma pack(pop)
}

void QuadVBO::prepare(GLVertexArray& vao)
{
    if (m_prepared) return;
    m_prepared = true;

    m_vbo.create();

    prepareVBO(vao);
}

void QuadVBO::prepareVBO(GLVertexArray& vao)
{
    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(TexVBO);
    {
        const int sz = stride_size * VERTEX_COUNT;

        TexVBO* buffer = (TexVBO*)new unsigned char[sz];
        memset(buffer, 0, sz);

        constexpr int row_size = ROW_SIZE;

        TexVBO* vbo = buffer;
        for (int i = 0; i < VERTEX_COUNT; i++) {
            int base = i * row_size;

            vbo->pos.x = VERTICES[base++];
            vbo->pos.y = VERTICES[base++];
            vbo->pos.z = VERTICES[base++];

            vbo->normal.x = (int)(VERTICES[base++] * ki::SCALE_VEC10);
            vbo->normal.y = (int)(VERTICES[base++] * ki::SCALE_VEC10);
            vbo->normal.z = (int)(VERTICES[base++] * ki::SCALE_VEC10);

            vbo->tangent.x = (int)(VERTICES[base++] * ki::SCALE_VEC10);
            vbo->tangent.y = (int)(VERTICES[base++] * ki::SCALE_VEC10);
            vbo->tangent.z = (int)(VERTICES[base++] * ki::SCALE_VEC10);

            vbo->texCoords.u = (int)(VERTICES[base++] * ki::SCALE_UV16);
            vbo->texCoords.v = (int)(VERTICES[base++] * ki::SCALE_UV16);

            vbo++;
        }

        glNamedBufferStorage(m_vbo, sz, buffer, 0);
        delete[] buffer;
    }

    glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, m_vbo, 0, stride_size);
    {
        glEnableVertexArrayAttrib(vao, ATTR_POS);
        glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
        glEnableVertexArrayAttrib(vao, ATTR_TANGENT);
        glEnableVertexArrayAttrib(vao, ATTR_TEX);

        // vertex attr
        glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(TexVBO, pos));

        // normal attr
        glVertexArrayAttribFormat(vao, ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(TexVBO, normal));

        // tangent attr
        glVertexArrayAttribFormat(vao, ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(TexVBO, tangent));

        // texture attr
        glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(TexVBO, texCoords));

        glVertexArrayAttribBinding(vao, ATTR_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_NORMAL, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TANGENT, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TEX, VBO_VERTEX_BINDING);
    }
}
