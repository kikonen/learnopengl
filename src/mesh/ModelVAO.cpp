#include "ModelVAO.h"

#include "glm/glm.hpp"
#include <fmt/format.h>

#include "asset/Shader.h"

#include "util/thread.h"

#include "kigl/GLState.h"

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
    ModelVAO::ModelVAO(std::string_view name)
        : m_name(name),
        m_positionVbo{ m_name + "_position_vbo" },
        m_normalVbo{ m_name + "_normal_vbo" },
        m_textureVbo{ m_name + "_texture_vbo" },
        m_ebo{ m_name + "_ebo" }
    {}

    ModelVAO::~ModelVAO() = default;

    void ModelVAO::prepare(std::string_view name)
    {
        if (m_prepared) return;
        m_prepared = true;

        {
            m_vao = std::make_unique<kigl::GLVertexArray>();
            m_vao->create(name);
        }
        {
            m_positionVbo.createEmpty(VERTEX_BLOCK_SIZE * sizeof(PositionEntry), GL_DYNAMIC_STORAGE_BIT);

            m_positionEntries.reserve(VERTEX_BLOCK_SIZE);
        }
        {
            m_normalVbo.createEmpty(VERTEX_BLOCK_SIZE * sizeof(NormalEntry), GL_DYNAMIC_STORAGE_BIT);

            m_normalEntries.reserve(VERTEX_BLOCK_SIZE);
        }
        {
            m_textureVbo.createEmpty(VERTEX_BLOCK_SIZE * sizeof(TextureEntry), GL_DYNAMIC_STORAGE_BIT);

            m_textureEntries.reserve(VERTEX_BLOCK_SIZE);
        }
        {
            m_ebo.createEmpty(INDEX_BLOCK_SIZE * sizeof(IndexEntry), GL_DYNAMIC_STORAGE_BIT);

            m_indexEntries.reserve(INDEX_BLOCK_SIZE);
        }

        // NOTE KI VBO & EBO are just empty buffers here

        prepareVAO(*m_vao, m_positionVbo, m_normalVbo, m_textureVbo, m_ebo);
    }

    void ModelVAO::bind()
    {
        kigl::GLState::get().bindVAO(*m_vao);
    }

    void ModelVAO::unbind()
    {
        kigl::GLState::get().bindVAO(0);
    }

    void ModelVAO::prepareVAO(
        kigl::GLVertexArray& vao,
        kigl::GLBuffer& positionVbo,
        kigl::GLBuffer& normalVbo,
        kigl::GLBuffer& textureVbo,
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
                glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, offsetof(PositionEntry, x));

                glVertexArrayAttribBinding(vao, ATTR_POS, VBO_POSITION_BINDING);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, VBO_POSITION_BINDING, 0);
            }
        }

        {
            glVertexArrayVertexBuffer(vao, VBO_NORMAL_BINDING, normalVbo, 0, sizeof(NormalEntry));
            {
                glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
                glEnableVertexArrayAttrib(vao, ATTR_TANGENT);

                // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                //

                // normal attr
                glVertexArrayAttribFormat(vao, ATTR_NORMAL, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(NormalEntry, normal));

                // tangent attr
                glVertexArrayAttribFormat(vao, ATTR_TANGENT, 4, GL_INT_2_10_10_10_REV, GL_TRUE, offsetof(NormalEntry, tangent));

                glVertexArrayAttribBinding(vao, ATTR_NORMAL, VBO_NORMAL_BINDING);
                glVertexArrayAttribBinding(vao, ATTR_TANGENT, VBO_NORMAL_BINDING);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, VBO_NORMAL_BINDING, 0);
            }
        }

        {
            glVertexArrayVertexBuffer(vao, VBO_TEXTURE_BINDING, textureVbo, 0, sizeof(TextureEntry));
            {
                glEnableVertexArrayAttrib(vao, ATTR_TEX);

                // https://stackoverflow.com/questions/37972229/glvertexattribpointer-and-glvertexattribformat-whats-the-difference
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                //

                // texture attr
                //glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_UNSIGNED_SHORT, GL_TRUE, offsetof(VertexEntry, texCoord));
                glVertexArrayAttribFormat(vao, ATTR_TEX, 2, GL_FLOAT, GL_FALSE, offsetof(TextureEntry, texCoord));

                glVertexArrayAttribBinding(vao, ATTR_TEX, VBO_TEXTURE_BINDING);

                // https://community.khronos.org/t/direct-state-access-instance-attribute-buffer-specification/75611
                // https://www.khronos.org/opengl/wiki/Vertex_Specification
                glVertexArrayBindingDivisor(vao, VBO_TEXTURE_BINDING, 0);
            }
        }

        {
            glVertexArrayElementBuffer(vao, ebo);
        }
    }

    void ModelVAO::clear()
    {
        m_positionEntries.clear();
        m_normalEntries.clear();
        m_textureEntries.clear();
        m_indexEntries.clear();

        m_lastPositionSize = 0;
        m_lastNormalSize = 0;
        m_lastTextureSize = 0;
        m_lastIndexSize = 0;
    }

    kigl::GLVertexArray* ModelVAO::registerModel(ModelVBO& modelVBO)
    {
        ASSERT_RT();
        assert(!modelVBO.m_positionEntries.empty());
        assert(!modelVBO.m_normalEntries.empty());
        assert(!modelVBO.m_textureEntries.empty());
        assert(!modelVBO.m_indexEntries.empty());
        assert(modelVBO.m_positionEntries.size() == modelVBO.m_normalEntries.size());
        assert(modelVBO.m_positionEntries.size() == modelVBO.m_textureEntries.size());

        {
            const size_t baseIndex = m_positionEntries.size();
            const size_t baseOffset = baseIndex * sizeof(PositionEntry);

            modelVBO.m_positionVboOffset = baseOffset;
        }

        {
            const size_t count = modelVBO.m_positionEntries.size();

            if (m_positionEntries.size() + count >= MAX_VERTEX_COUNT)
                throw std::runtime_error{ fmt::format("MAX_VERTEX_COUNT: {}", MAX_VERTEX_COUNT) };

            {
                size_t size = m_positionEntries.size() + std::max(VERTEX_BLOCK_SIZE, count) + VERTEX_BLOCK_SIZE;
                size += VERTEX_BLOCK_SIZE - size % VERTEX_BLOCK_SIZE;
                size = std::min(size, MAX_VERTEX_COUNT);
                m_positionEntries.reserve(size);
            }

            auto base = m_positionEntries.size();
            m_positionEntries.insert(
                m_positionEntries.end(),
                modelVBO.m_positionEntries.begin(),
                modelVBO.m_positionEntries.end());

            for (size_t i = 0; i < count; i++) {
                m_positionEntries[base + i] += modelVBO.m_meshPositionOffset;
            }
        }

        {
            const size_t count = modelVBO.m_normalEntries.size();

            if (m_normalEntries.size() + count >= MAX_VERTEX_COUNT)
                throw std::runtime_error{ fmt::format("MAX_VERTEX_COUNT: {}", MAX_VERTEX_COUNT) };

            {
                size_t size = m_normalEntries.size() + std::max(VERTEX_BLOCK_SIZE, count) + VERTEX_BLOCK_SIZE;
                size += VERTEX_BLOCK_SIZE - size % VERTEX_BLOCK_SIZE;
                size = std::min(size, MAX_VERTEX_COUNT);
                m_normalEntries.reserve(size);
            }

            m_normalEntries.insert(
                m_normalEntries.end(),
                modelVBO.m_normalEntries.begin(),
                modelVBO.m_normalEntries.end());
        }

        {
            const size_t count = modelVBO.m_textureEntries.size();

            if (m_textureEntries.size() + count >= MAX_VERTEX_COUNT)
                throw std::runtime_error{ fmt::format("MAX_VERTEX_COUNT: {}", MAX_VERTEX_COUNT) };

            {
                size_t size = m_textureEntries.size() + std::max(VERTEX_BLOCK_SIZE, count) + VERTEX_BLOCK_SIZE;
                size += VERTEX_BLOCK_SIZE - size % VERTEX_BLOCK_SIZE;
                size = std::min(size, MAX_VERTEX_COUNT);
                m_textureEntries.reserve(size);
            }

            m_textureEntries.insert(
                m_textureEntries.end(),
                modelVBO.m_textureEntries.begin(),
                modelVBO.m_textureEntries.end());
        }

        {
            const size_t count = modelVBO.m_indexEntries.size();
            const size_t baseIndex = m_indexEntries.size();
            const size_t baseOffset = baseIndex * sizeof(IndexEntry);

            if (m_indexEntries.size() + count >= MAX_INDEX_COUNT)
                throw std::runtime_error{ fmt::format("MAX_INDEX_COUNT: {}", MAX_INDEX_COUNT) };

            modelVBO.m_indexEboOffset = baseOffset;

            {
                size_t size = m_indexEntries.size() + std::max(INDEX_BLOCK_SIZE, count) + INDEX_BLOCK_SIZE;
                size += INDEX_BLOCK_SIZE - size % INDEX_BLOCK_SIZE;
                size = std::min(size, MAX_INDEX_COUNT);
                m_indexEntries.reserve(size);
            }

            m_indexEntries.insert(
                m_indexEntries.end(),
                modelVBO.m_indexEntries.begin(),
                modelVBO.m_indexEntries.end());
        }

        return m_vao.get();
    }

    void ModelVAO::updateRT()
    {
        ASSERT_RT();

        updatePositionBuffer();
        updateNormalBuffer();
        updateTextureBuffer();
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

            m_positionVbo.invalidateRange(
                updateIndex * sz,
                updateCount * sz);

            m_positionVbo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_positionEntries[updateIndex]);
        }

        m_lastPositionSize = totalCount;
    }

    void ModelVAO::updateNormalBuffer()
    {
        const size_t index = m_lastNormalSize;
        const size_t totalCount = m_normalEntries.size();

        if (index == totalCount) return;
        if (totalCount == 0) return;

        {
            constexpr size_t sz = sizeof(NormalEntry);
            size_t updateIndex = index;

            // NOTE KI *reallocate* SSBO if needed
            if (m_normalVbo.m_size < totalCount * sz) {
                m_normalVbo.resizeBuffer(m_normalEntries.capacity() * sz);
                glVertexArrayVertexBuffer(*m_vao, VBO_NORMAL_BINDING, m_normalVbo, 0, sizeof(NormalEntry));
                updateIndex = 0;
            }

            const size_t updateCount = totalCount - updateIndex;

            m_normalVbo.invalidateRange(
                updateIndex * sz,
                updateCount * sz);

            m_normalVbo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_normalEntries[updateIndex]);
        }

        m_lastNormalSize = totalCount;
    }

    void ModelVAO::updateTextureBuffer()
    {
        const size_t index = m_lastTextureSize;
        const size_t totalCount = m_textureEntries.size();

        if (index == totalCount) return;
        if (totalCount == 0) return;

        {
            constexpr size_t sz = sizeof(TextureEntry);
            size_t updateIndex = index;

            // NOTE KI *reallocate* SSBO if needed
            if (m_textureVbo.m_size < totalCount * sz) {
                m_textureVbo.resizeBuffer(m_textureEntries.capacity() * sz);
                glVertexArrayVertexBuffer(*m_vao, VBO_TEXTURE_BINDING, m_textureVbo, 0, sizeof(TextureEntry));
                updateIndex = 0;
            }

            const size_t updateCount = totalCount - updateIndex;

            m_textureVbo.invalidateRange(
                updateIndex * sz,
                updateCount * sz);

            m_textureVbo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_textureEntries[updateIndex]);
        }

        m_lastTextureSize = totalCount;
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

            m_ebo.invalidateRange(
                updateIndex * sz,
                updateCount * sz);

            m_ebo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_indexEntries[updateIndex]);
        }

        m_lastIndexSize = totalCount;

    }
}
