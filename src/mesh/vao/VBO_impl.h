#pragma once

#include "VBO.h"

#include "kigl/GLVertexArray.h"

namespace mesh {
    constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    constexpr size_t MAX_VERTEX_BLOCK_COUNT = 50000;

    constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * MAX_VERTEX_BLOCK_COUNT;

    template<typename T_Vertex, typename T_Entry>
    VBO<T_Vertex, T_Entry>::VBO(
        std::string_view name,
        int attr,
        int binding)
        : m_vbo{ name },
        m_attr{ attr },
        m_binding{ binding }
    {
        clear();
    }

    template<typename T_Vertex, typename T_Entry>
    VBO<T_Vertex, T_Entry>::~VBO()
    {
    }

    template<typename T_Vertex, typename T_Entry>
    void VBO<T_Vertex, T_Entry>::clear()
    {
        m_entries.clear();
        m_dirty.clear();

        // NOTE KI reserve 0 for NULL
        reserveVertices(1);

        m_vbo.markUsed(0);
    }

    template<typename T_Vertex, typename T_Entry>
    uint32_t VBO<T_Vertex, T_Entry>::reserveVertices(size_t count)
    {
        const uint32_t baseIndex = static_cast<uint32_t>(m_entries.size());

        reserveSize(count);
        m_entries.resize(m_entries.size() + count);

        return baseIndex;
    }

    template<typename T_Vertex, typename T_Entry>
    void VBO<T_Vertex, T_Entry>::updateVertices(
        uint32_t baseIndex,
        const std::span<T_Vertex>& vertices)
    {
        assert(baseIndex + vertices.size() <= m_entries.size());

        uint32_t index = baseIndex;
        for (const auto& vertex : vertices) {
            m_entries[index] = convertVertex(vertex);
            index++;
        }

        m_dirty.push_back({baseIndex, vertices.size()});
    }

    template<typename T_Vertex, typename T_Entry>
    void VBO<T_Vertex, T_Entry>::reserveSize(size_t count)
    {
        size_t size = m_entries.size() + std::max(VERTEX_BLOCK_SIZE, count) + VERTEX_BLOCK_SIZE;
        size += VERTEX_BLOCK_SIZE - size % VERTEX_BLOCK_SIZE;
        if (size > MAX_VERTEX_COUNT) {
            KI_CRITICAL(fmt::format("ERROR: MAX_VERTEX_COUNT reached, size={}", size));
            size = std::min(size, MAX_VERTEX_COUNT);
        }
        m_entries.reserve(size);
    }

    template<typename T_Vertex, typename T_Entry>
    void VBO<T_Vertex, T_Entry>::updateVAO(kigl::GLVertexArray& vao)
    {
        if (m_dirty.empty()) return;

        for (const auto& range : m_dirty) {
            if (updateSpan(vao, range.first, range.second)) break;
        }

        m_dirty.clear();
    }

    template<typename T_Vertex, typename T_Entry>
    bool VBO<T_Vertex, T_Entry>::updateSpan(
        kigl::GLVertexArray & vao,
        size_t updateIndex,
        size_t updateCount)
    {
        const size_t totalCount = m_entries.size();

        if (totalCount == 0) return true;

        {
            constexpr size_t sz = sizeof(T_Entry);

            // NOTE KI *reallocate* SSBO if needed
            if (m_vbo.size() < totalCount * sz) {
                m_vbo.resizeBuffer(m_entries.capacity() * sz, true);
                glVertexArrayVertexBuffer(vao, m_binding, m_vbo, 0, sz);

                //updateIndex = 0;
                //updateCount = totalCount;
            }

            //m_vbo.invalidateRange(
            //    updateIndex * sz,
            //    updateCount * sz);

            m_vbo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_entries[updateIndex]);

            m_vbo.markUsed(totalCount * sz);
        }

        return updateCount == totalCount;
    }
}
