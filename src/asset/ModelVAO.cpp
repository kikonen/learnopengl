#include "ModelVAO.h"

#include "glm/glm.hpp"
#include <fmt/format.h>

#include "asset/Program.h"
#include "asset/Shader.h"

#include "asset/ModelMeshVBO.h"

namespace {
    constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    constexpr size_t VERTEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * VERTEX_BLOCK_COUNT;

    constexpr size_t INDEX_BLOCK_SIZE = 1000;
    constexpr size_t INDEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_INDEX_COUNT = INDEX_BLOCK_SIZE * INDEX_BLOCK_COUNT;
}

ModelVAO::ModelVAO(bool singleMaterial)
    : m_singleMaterial(singleMaterial)
{
}


GLVertexArray* ModelVAO::prepare()
{
    if (m_prepared) return m_vao.get();
    m_prepared = true;

    {
        m_vao = std::make_unique<GLVertexArray>();
        m_vao->create();
    }
    {
        m_vbo.createEmpty(VERTEX_BLOCK_SIZE * sizeof(VertexEntry), GL_DYNAMIC_STORAGE_BIT);

        m_vertexEntries.reserve(VERTEX_BLOCK_SIZE);
    }
    {
        m_ebo.createEmpty(INDEX_BLOCK_SIZE * sizeof(IndexEntry), GL_DYNAMIC_STORAGE_BIT);

        m_indexEntries.reserve(INDEX_BLOCK_SIZE);
    }

    // NOTE KI VBO & EBO are just empty buffers here

    prepareVAO(*m_vao, m_vbo, m_ebo);
    return m_vao.get();
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
        const size_t count = meshVBO.m_vertexEntries.size();
        const size_t baseIndex = m_vertexEntries.size();
        const size_t baseOffset = baseIndex * sizeof(VertexEntry);

        if (m_vertexEntries.size() + count >= MAX_VERTEX_COUNT)
            throw std::runtime_error{ fmt::format("MAX_VERTEX_COUNT: {}", MAX_VERTEX_COUNT) };

        meshVBO.m_vertexOffset = baseOffset;

        {
            size_t size = m_vertexEntries.size() + std::max(VERTEX_BLOCK_SIZE, count) + VERTEX_BLOCK_SIZE;
            size += VERTEX_BLOCK_SIZE - size % VERTEX_BLOCK_SIZE;
            size = std::min(size, MAX_VERTEX_COUNT);
            m_vertexEntries.reserve(size);
        }

        m_vertexEntries.insert(
            m_vertexEntries.end(),
            meshVBO.m_vertexEntries.begin(),
            meshVBO.m_vertexEntries.end());
    }

    {
        const size_t count = meshVBO.m_indexEntries.size();
        const size_t baseIndex = m_indexEntries.size();
        const size_t baseOffset = baseIndex * sizeof(IndexEntry);

        if (m_indexEntries.size() + count >= MAX_INDEX_COUNT)
            throw std::runtime_error{ fmt::format("MAX_INDEX_COUNT: {}", MAX_INDEX_COUNT) };

        meshVBO.m_indexOffset = baseOffset;

        {
            size_t size = m_indexEntries.size() + std::max(INDEX_BLOCK_SIZE, count) + INDEX_BLOCK_SIZE;
            size += INDEX_BLOCK_SIZE - size % INDEX_BLOCK_SIZE;
            size = std::min(size, MAX_INDEX_COUNT);
            m_indexEntries.reserve(size);
        }

        m_indexEntries.insert(
            m_indexEntries.end(),
            meshVBO.m_indexEntries.begin(),
            meshVBO.m_indexEntries.end());
    }

    return m_vao.get();
}

void ModelVAO::update(const UpdateContext& ctx)
{
    updateVertexBuffer();
    updateIndexBuffer();
}

void ModelVAO::updateVertexBuffer()
{
    const size_t index = m_lastVertexSize;
    const size_t totalCount = m_vertexEntries.size();

    if (index == totalCount) return;
    if (totalCount == 0) return;

    {
        constexpr size_t sz = sizeof(VertexEntry);
        int updateIndex = index;

        // NOTE KI *reallocate* SSBO if needed
        if (m_vbo.m_size < totalCount * sz) {
            m_vbo.resizeBuffer(m_vertexEntries.capacity() * sz);
            glVertexArrayVertexBuffer(*m_vao, VBO_VERTEX_BINDING, m_vbo, 0, sizeof(VertexEntry));
            updateIndex = 0;
        }

        const int updateCount = totalCount - index;

        m_vbo.update(
            updateIndex * sz,
            updateCount * sz,
            &m_vertexEntries[updateIndex]);
    }

    m_lastVertexSize = totalCount;
}

void ModelVAO::updateIndexBuffer()
{
    const size_t index = m_lastIndexSize;
    const size_t totalCount = m_indexEntries.size();

    if (index == totalCount) return;
    if (totalCount == 0) return;

    {
        constexpr size_t sz = sizeof(IndexEntry);
        int updateIndex = index;

        // NOTE KI *reallocate* SSBO if needed
        if (m_ebo.m_size < totalCount * sz) {
            m_ebo.resizeBuffer(m_indexEntries.capacity() * sz);
            glVertexArrayElementBuffer(*m_vao, m_ebo);
            updateIndex = 0;
        }

        const int updateCount = totalCount - index;

        m_ebo.update(
            updateIndex * sz,
            updateCount * sz,
            &m_indexEntries[updateIndex]);
    }

    m_lastIndexSize = totalCount;
}
