#include "ModelVAO.h"

#include "glm/glm.hpp"
#include <fmt/format.h>

#include "asset/Shader.h"

#include "mesh/ModelVBO.h"

namespace {
    constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    constexpr size_t VERTEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * VERTEX_BLOCK_COUNT;

    constexpr size_t INDEX_BLOCK_SIZE = 1000;
    constexpr size_t INDEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_INDEX_COUNT = INDEX_BLOCK_SIZE * INDEX_BLOCK_COUNT;
}

namespace mesh {
    void ModelVAO::prepare()
    {
        if (m_prepared) return;
        m_prepared = true;

        {
            m_vao = std::make_unique<kigl::GLVertexArray>();
            m_vao->create("model");
        }
        {
            m_positionVbo.createEmpty(VERTEX_BLOCK_SIZE * sizeof(PositionEntry), GL_DYNAMIC_STORAGE_BIT);

            m_positionEntries.reserve(VERTEX_BLOCK_SIZE);
        }
        {
            m_vertexVbo.createEmpty(VERTEX_BLOCK_SIZE * sizeof(VertexEntry), GL_DYNAMIC_STORAGE_BIT);

            m_vertexEntries.reserve(VERTEX_BLOCK_SIZE);
        }
        {
            m_ebo.createEmpty(INDEX_BLOCK_SIZE * sizeof(IndexEntry), GL_DYNAMIC_STORAGE_BIT);

            m_indexEntries.reserve(INDEX_BLOCK_SIZE);
        }

        // NOTE KI VBO & EBO are just empty buffers here

        prepareVAO(*m_vao, m_positionVbo, m_vertexVbo, m_ebo);
    }

    void ModelVAO::prepareVAO(
        kigl::GLVertexArray& vao,
        kigl::GLBuffer& positionVbo,
        kigl::GLBuffer& vertexVbo,
        kigl::GLBuffer& ebo)
    {
        // "Tile" based GPU can benefit from having separate position stream VBO for improved caching
        // https://solidpixel.github.io/2022/07/21/vertexpacking.html
        // https://www.intel.com/content/www/us/en/developer/articles/guide/developer-and-optimization-guide-for-intel-processor-graphics-gen11-api.html

        {
            glVertexArrayVertexBuffer(vao, VBO_POSITION_BINDING, positionVbo, 0, sizeof(PositionEntry));
            {
                glEnableVertexArrayAttrib(vao, ATTR_POS);

                // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                //
                // vertex attr
                glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(PositionEntry, pos));

                glVertexArrayAttribBinding(vao, ATTR_POS, VBO_POSITION_BINDING);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, VBO_POSITION_BINDING, 0);
            }
        }

        {
            glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, vertexVbo, 0, sizeof(VertexEntry));
            {
                glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
                glEnableVertexArrayAttrib(vao, ATTR_TANGENT);
                glEnableVertexArrayAttrib(vao, ATTR_TEX);

                // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                //

                // normal attr
                glVertexArrayAttribFormat(vao, ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexEntry, normal));

                // tangent attr
                glVertexArrayAttribFormat(vao, ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(VertexEntry, tangent));

                // texture attr
                glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, texCoord));

                glVertexArrayAttribBinding(vao, ATTR_NORMAL, VBO_VERTEX_BINDING);
                glVertexArrayAttribBinding(vao, ATTR_TANGENT, VBO_VERTEX_BINDING);
                glVertexArrayAttribBinding(vao, ATTR_TEX, VBO_VERTEX_BINDING);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, VBO_VERTEX_BINDING, 0);
            }
        }

        {
            glVertexArrayElementBuffer(vao, ebo);
        }
    }

    kigl::GLVertexArray* ModelVAO::registerModel(ModelVBO& meshVBO)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        assert(!meshVBO.m_positionEntries.empty());
        assert(!meshVBO.m_vertexEntries.empty());
        assert(!meshVBO.m_indexEntries.empty());

        {
            const size_t count = meshVBO.m_positionEntries.size();
            const size_t baseIndex = m_positionEntries.size();
            const size_t baseOffset = baseIndex * sizeof(PositionEntry);

            if (m_positionEntries.size() + count >= MAX_VERTEX_COUNT)
                throw std::runtime_error{ fmt::format("MAX_VERTEX_COUNT: {}", MAX_VERTEX_COUNT) };

            meshVBO.m_positionOffset = baseOffset;

            {
                size_t size = m_positionEntries.size() + std::max(VERTEX_BLOCK_SIZE, count) + VERTEX_BLOCK_SIZE;
                size += VERTEX_BLOCK_SIZE - size % VERTEX_BLOCK_SIZE;
                size = std::min(size, MAX_VERTEX_COUNT);
                m_positionEntries.reserve(size);
            }

            m_positionEntries.insert(
                m_positionEntries.end(),
                meshVBO.m_positionEntries.begin(),
                meshVBO.m_positionEntries.end());
        }

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

    void ModelVAO::updateRT(const UpdateContext& ctx)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        updatePositionBuffer();
        updateVertexBuffer();
        updateIndexBuffer();
    }

    void ModelVAO::updatePositionBuffer()
    {
        const size_t index = m_lastPositionSize;
        const size_t totalCount = m_positionEntries.size();

        if (index == totalCount) return;
        if (totalCount == 0) return;

        {
            constexpr size_t sz = sizeof(PositionEntry);
            size_t updateIndex = index;

            // NOTE KI *reallocate* SSBO if needed
            if (m_positionVbo.m_size < totalCount * sz) {
                m_positionVbo.resizeBuffer(m_positionEntries.capacity() * sz);
                glVertexArrayVertexBuffer(*m_vao, VBO_POSITION_BINDING, m_positionVbo, 0, sizeof(PositionEntry));
                updateIndex = 0;
            }

            const size_t updateCount = totalCount - updateIndex;

            m_positionVbo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_positionEntries[updateIndex]);
        }

        m_lastPositionSize = totalCount;
    }

    void ModelVAO::updateVertexBuffer()
    {
        const size_t index = m_lastVertexSize;
        const size_t totalCount = m_vertexEntries.size();

        if (index == totalCount) return;
        if (totalCount == 0) return;

        {
            constexpr size_t sz = sizeof(VertexEntry);
            size_t updateIndex = index;

            // NOTE KI *reallocate* SSBO if needed
            if (m_vertexVbo.m_size < totalCount * sz) {
                m_vertexVbo.resizeBuffer(m_vertexEntries.capacity() * sz);
                glVertexArrayVertexBuffer(*m_vao, VBO_VERTEX_BINDING, m_vertexVbo, 0, sizeof(VertexEntry));
                updateIndex = 0;
            }

            const size_t updateCount = totalCount - updateIndex;

            m_vertexVbo.update(
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
            size_t updateIndex = index;

            // NOTE KI *reallocate* SSBO if needed
            if (m_ebo.m_size < totalCount * sz) {
                m_ebo.resizeBuffer(m_indexEntries.capacity() * sz);
                glVertexArrayElementBuffer(*m_vao, m_ebo);
                updateIndex = 0;
            }

            const size_t updateCount = totalCount - updateIndex;

            m_ebo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_indexEntries[updateIndex]);
        }

        m_lastIndexSize = totalCount;
    }
}
