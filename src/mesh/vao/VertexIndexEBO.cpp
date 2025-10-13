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

    void VertexIndexEBO::clear()
    {
        m_entries.clear();
        m_dirty.clear();

        m_ebo.markUsed(0);
    }


    uint32_t VertexIndexEBO::reserveIndeces(size_t count)
    {
        if (count == 0) return 0;

        const size_t baseIndex = m_entries.size();

        if (m_entries.size() + count >= MAX_INDEX_COUNT)
            throw std::runtime_error{ fmt::format("MAX_INDEX_COUNT: {}", MAX_INDEX_COUNT) };

        {
            size_t size = m_entries.size() + std::max(INDEX_BLOCK_SIZE, static_cast<size_t>(count)) + INDEX_BLOCK_SIZE;
            size += INDEX_BLOCK_SIZE - size % INDEX_BLOCK_SIZE;
            size = std::min(size, MAX_INDEX_COUNT);
            m_entries.reserve(size);
        }

        m_entries.resize(m_entries.size() + count);

        return static_cast<uint32_t>(baseIndex);
    }

    void VertexIndexEBO::updateIndeces(
        uint32_t baseIndex,
        std::span<mesh::Index32> indeces) noexcept
    {
        assert(baseIndex + indeces.size() <= m_entries.size());

        if (indeces.size() == 0) return;

        std::copy(
            indeces.begin(),
            indeces.end(),
            m_entries.begin() + baseIndex);

        m_dirty.push_back({ baseIndex, indeces.size() });
    }

    void VertexIndexEBO::prepareVAO(kigl::GLVertexArray& vao)
    {
        {
            m_entries.reserve(INDEX_BLOCK_SIZE);
            m_ebo.createEmpty(INDEX_BLOCK_SIZE * sizeof(IndexEntry32), GL_DYNAMIC_STORAGE_BIT);
        }
        glVertexArrayElementBuffer(vao, m_ebo);
    }

    void VertexIndexEBO::updateVAO(kigl::GLVertexArray& vao)
    {
        if (m_dirty.empty()) return;

        for (const auto& range : m_dirty) {
            if (updateSpan(vao, range.first, range.second)) break;
        }

        m_dirty.clear();
    }

    bool VertexIndexEBO::updateSpan(
        kigl::GLVertexArray& vao,
        size_t updateIndex,
        size_t updateCount)
    {
        const size_t totalCount = m_entries.size();

        if (totalCount == 0) return true;

        {
            constexpr size_t sz = sizeof(IndexEntry32);

            // NOTE KI *reallocate* SSBO if needed
            if (m_ebo.m_size < totalCount * sz) {
                m_ebo.resizeBuffer(m_entries.capacity() * sz, true);
                glVertexArrayElementBuffer(vao, m_ebo);

                //updateIndex = 0;
                //updateCount = totalCount;
            }

            //m_ebo.invalidateRange(
            //    updateIndex * sz,
            //    updateCount * sz);

            m_ebo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_entries[updateIndex]);

            m_ebo.markUsed(totalCount * sz);
        }

        return updateCount == totalCount;
    }
}
