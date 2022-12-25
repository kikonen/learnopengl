#include "QuadVAO.h"

#include "glm/glm.hpp"

#include "Shader.h"

#include "scene/Batch.h"

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
    struct VertexEntry {
        glm::vec3 pos;
        ki::VEC10 normal;
        ki::VEC10 tangent;
        ki::UV16 texCoords;
    };
#pragma pack(pop)
}

GLVertexArray* QuadVAO::prepare(Batch& batch)
{
    if (m_prepared) return m_vao.get();
    m_prepared = true;

    m_vao = std::make_unique<GLVertexArray>();
    m_vao->create();
    m_vbo.create();

    prepareVBO(m_vbo);
    prepareVAO(*m_vao, m_vbo);

    batch.prepareVAO(*m_vao, true);

    return m_vao.get();
}

void QuadVAO::prepareVAO(
    GLVertexArray& vao,
    GLBuffer& vbo)
{
    glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, vbo, 0, sizeof(VertexEntry));
    {
        glEnableVertexArrayAttrib(vao, ATTR_POS);
        glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
        glEnableVertexArrayAttrib(vao, ATTR_TANGENT);
        glEnableVertexArrayAttrib(vao, ATTR_TEX);

        // vertex attr
        glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(VertexEntry, pos));

        // normal attr
        glVertexArrayAttribFormat(vao, ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexEntry, normal));

        // tangent attr
        glVertexArrayAttribFormat(vao, ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexEntry, tangent));

        // texture attr
        glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, texCoords));

        glVertexArrayAttribBinding(vao, ATTR_POS, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_NORMAL, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TANGENT, VBO_VERTEX_BINDING);
        glVertexArrayAttribBinding(vao, ATTR_TEX, VBO_VERTEX_BINDING);
    }
}

void QuadVAO::prepareVBO(GLBuffer& vbo)
{
    // https://paroj.github.io/gltut/Basic%20Optimization.html
    constexpr int stride_size = sizeof(VertexEntry);
    const int sz = stride_size * VERTEX_COUNT;

    VertexEntry* buffer = (VertexEntry*)new unsigned char[sz];
    memset(buffer, 0, sz);

    constexpr int row_size = ROW_SIZE;

    VertexEntry* entry = buffer;
    for (int i = 0; i < VERTEX_COUNT; i++) {
        int base = i * row_size;

        entry->pos.x = VERTICES[base++];
        entry->pos.y = VERTICES[base++];
        entry->pos.z = VERTICES[base++];

        entry->normal.x = (int)(VERTICES[base++] * ki::SCALE_VEC10);
        entry->normal.y = (int)(VERTICES[base++] * ki::SCALE_VEC10);
        entry->normal.z = (int)(VERTICES[base++] * ki::SCALE_VEC10);

        entry->tangent.x = (int)(VERTICES[base++] * ki::SCALE_VEC10);
        entry->tangent.y = (int)(VERTICES[base++] * ki::SCALE_VEC10);
        entry->tangent.z = (int)(VERTICES[base++] * ki::SCALE_VEC10);

        entry->texCoords.u = (int)(VERTICES[base++] * ki::SCALE_UV16);
        entry->texCoords.v = (int)(VERTICES[base++] * ki::SCALE_UV16);

        entry++;
    }

    glNamedBufferStorage(vbo, sz, buffer, 0);
    delete[] buffer;
}
