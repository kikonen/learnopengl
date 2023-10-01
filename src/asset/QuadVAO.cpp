#include "QuadVAO.h"

#include "glm/glm.hpp"

#include "asset/Program.h"
#include "asset/Shader.h"

#include "render/Batch.h"

namespace {
    // NOTE KI normal, tangent, tex stored to allow normal g_tex shader
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
        ki::UV16 texCoord;
    };
#pragma pack(pop)
}

GLVertexArray* QuadVAO::prepare()
{
    if (m_prepared) return m_vao.get();
    m_prepared = true;

    m_vao = std::make_unique<GLVertexArray>();
    m_vao->create("quad");
    m_vbo.create();

    prepareVBO(m_vbo);
    prepareVAO(*m_vao, m_vbo);

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
        glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, texCoord));

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

        entry->pos = { VERTICES[base++], VERTICES[base++], VERTICES[base++] };
        entry->normal = { VERTICES[base++], VERTICES[base++], VERTICES[base++] };
        entry->tangent = { VERTICES[base++], VERTICES[base++], VERTICES[base++] };
        entry->texCoord = { VERTICES[base++], VERTICES[base++] };

        entry++;
    }

    vbo.init(sz, buffer, 0);
    delete[] buffer;
}
