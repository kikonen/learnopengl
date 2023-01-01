#include "ModelVAO.h"

#include "glm/glm.hpp"
#include <fmt/format.h>

#include "Shader.h"

#include "asset/ModelMeshVBO.h"

#include "scene/Batch.h"

namespace {
    constexpr int MAX_VERTEX_ENTRIES = 100000;
    constexpr int MAX_INDEX_ENTRIES = 100000;
}

ModelVAO::ModelVAO(bool singleMaterial)
    : m_singleMaterial(singleMaterial)
{
}


GLVertexArray* ModelVAO::prepare(Batch& batch)
{
    if (m_prepared) return m_vao.get();
    m_prepared = true;

    {
        m_vao = std::make_unique<GLVertexArray>();
        m_vao->create();
    }
    {
        m_vbo.create();
        m_vbo.initEmpty(MAX_VERTEX_ENTRIES * sizeof(VertexEntry), GL_DYNAMIC_STORAGE_BIT);

        m_vertexEntries.reserve(MAX_VERTEX_ENTRIES);
    }
    {
        m_ebo.create();
        m_ebo.initEmpty(MAX_INDEX_ENTRIES * sizeof(IndexEntry), GL_DYNAMIC_STORAGE_BIT);

        m_indexEntries.reserve(MAX_INDEX_ENTRIES);
    }

    // NOTE KI VBO & EBO are just empty buffers here

    prepareVAO(*m_vao, m_vbo, m_ebo);
    batch.prepareVAO(*m_vao, m_singleMaterial);
}

void ModelVAO::prepareVAO(
    GLVertexArray& vao,
    GLBuffer& vbo,
    GLBuffer& ebo)
{
    glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, vbo, 0, sizeof(VertexEntry));
    {
        glEnableVertexArrayAttrib(vao, ATTR_POS);
        glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
        glEnableVertexArrayAttrib(vao, ATTR_TANGENT);
        glEnableVertexArrayAttrib(vao, ATTR_TEX);

        // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        //
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

        // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
        // https://www.khronos.org/opengl/wiki/Vertex_Specification
        glVertexArrayBindingDivisor(vao, VBO_VERTEX_BINDING, 0);
    }
    {
        glVertexArrayElementBuffer(vao, ebo);
    }
}

GLVertexArray* ModelVAO::registerModel(ModelMeshVBO& meshVBO)
{
    assert(!meshVBO.m_vertexEntries.empty());
    assert(!meshVBO.m_indexEntries.empty());

    {
        const int count = meshVBO.m_vertexEntries.size();
        const int baseIndex = m_vertexEntries.size();
        const int baseOffset = baseIndex * sizeof(VertexEntry);

        assert(baseIndex + count <= MAX_VERTEX_ENTRIES);

        KI_INFO(fmt::format("MESH vertex index={}, MAX={}", baseIndex, MAX_VERTEX_ENTRIES));

        meshVBO.m_vertexOffset = baseOffset;

        m_vertexEntries.insert(
            m_vertexEntries.end(),
            meshVBO.m_vertexEntries.begin(),
            meshVBO.m_vertexEntries.end());

        m_vbo.update(
            baseOffset,
            count * sizeof(VertexEntry),
            &m_vertexEntries[baseIndex]);
    }

    {
        const int count = meshVBO.m_indexEntries.size();
        const int baseIndex = m_indexEntries.size();
        const int baseOffset = baseIndex * sizeof(IndexEntry);

        assert(baseIndex + count <= MAX_INDEX_ENTRIES);

        KI_INFO(fmt::format("MESH index index={}, MAX={}", baseIndex, MAX_INDEX_ENTRIES));

        meshVBO.m_indexOffset = baseOffset;

        m_indexEntries.insert(
            m_indexEntries.end(),
            meshVBO.m_indexEntries.begin(),
            meshVBO.m_indexEntries.end());

        m_ebo.update(
            baseOffset,
            count * sizeof(IndexEntry),
            &m_indexEntries[baseIndex]);
    }

    return m_vao.get();
}
