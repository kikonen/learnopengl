#include "VertexIndexEBO.h"

#include "kigl/GLVertexArray.h"

namespace {
    constexpr size_t INDEX_BLOCK_SIZE = 1000;
    constexpr size_t INDEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_INDEX_COUNT = INDEX_BLOCK_SIZE * INDEX_BLOCK_COUNT;
}

namespace mesh {
    VertexIndexEBO::VertexIndexEBO(
        std::string_view name)
        : m_ebo{ name }
    {}

    size_t VertexIndexEBO::addIndeces(std::vector<mesh::Index> indeces)
    {
        const size_t count = indeces.size();
        const size_t baseIndex = m_entries.size();
        const size_t baseOffset = baseIndex * sizeof(IndexEntry);

        if (m_entries.size() + count >= MAX_INDEX_COUNT)
            throw std::runtime_error{ fmt::format("MAX_INDEX_COUNT: {}", MAX_INDEX_COUNT) };

        baseOffset;

        {
            size_t size = m_entries.size() + std::max(INDEX_BLOCK_SIZE, count) + INDEX_BLOCK_SIZE;
            size += INDEX_BLOCK_SIZE - size % INDEX_BLOCK_SIZE;
            size = std::min(size, MAX_INDEX_COUNT);
            m_entries.reserve(size);
        }

        m_entries.insert(
            m_entries.end(),
            indeces.begin(),
            indeces.end());

        return baseOffset;
    }

    void VertexIndexEBO::clear()
    {
        m_entries.clear();
        m_lastSize = 0;
    }

    void VertexIndexEBO::prepareVAO(kigl::GLVertexArray& vao)
    {
        {
            m_ebo.createEmpty(INDEX_BLOCK_SIZE * sizeof(IndexEntry), GL_DYNAMIC_STORAGE_BIT);
            m_entries.reserve(INDEX_BLOCK_SIZE);
        }
        glVertexArrayElementBuffer(vao, m_ebo);
    }

    void VertexIndexEBO::updateVAO(kigl::GLVertexArray& vao)
    {
        const size_t index = m_lastSize;
        const size_t totalCount = m_entries.size();

        if (index == totalCount) return;
        if (totalCount == 0) return;

        {
            constexpr size_t sz = sizeof(IndexEntry);
            size_t updateIndex = index;

            // NOTE KI *reallocate* SSBO if needed
            if (m_ebo.m_size < totalCount * sz) {
                m_ebo.resizeBuffer(m_entries.capacity() * sz);
                glVertexArrayElementBuffer(vao, m_ebo);
                updateIndex = 0;
            }

            const size_t updateCount = totalCount - updateIndex;

            //m_ebo.invalidateRange(
            //    updateIndex * sz,
            //    updateCount * sz);

            m_ebo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_entries[updateIndex]);
        }

        m_lastSize = totalCount;
    }
}
