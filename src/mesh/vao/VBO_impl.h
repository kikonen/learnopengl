#pragma once

#include "VBO.h"

#include "kigl/GLVertexArray.h"

namespace mesh {
    constexpr size_t VERTEX_BLOCK_SIZE = 1000;
    constexpr size_t VERTEX_BLOCK_COUNT = 10000;

    constexpr size_t MAX_VERTEX_COUNT = VERTEX_BLOCK_SIZE * VERTEX_BLOCK_COUNT;

    template<typename T_Vertex, typename T_Entry>
    VBO<T_Vertex, T_Entry>::VBO(
        std::string_view name,
        int attr,
        int binding)
        : m_vbo{ name },
        m_attr{ attr },
        m_binding{ binding }
    {}

    template<typename T_Vertex, typename T_Entry>
    VBO<T_Vertex, T_Entry>::~VBO()
    {}

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
        uint32_t index = baseIndex;
        for (const auto& vertex : vertices) {
            m_entries[index] = convertVertex(vertex);
            index++;
        }
    }

    template<typename T_Vertex, typename T_Entry>
    void VBO<T_Vertex, T_Entry>::reserveSize(size_t count)
    {
        size_t size = m_entries.size() + std::max(VERTEX_BLOCK_SIZE, count) + VERTEX_BLOCK_SIZE;
        size += VERTEX_BLOCK_SIZE - size % VERTEX_BLOCK_SIZE;
        size = std::min(size, MAX_VERTEX_COUNT);
        m_entries.reserve(size);
    }

    template<typename T_Vertex, typename T_Entry>
    void VBO<T_Vertex, T_Entry>::updateVAO(kigl::GLVertexArray& vao)
    {
        const size_t index = m_lastBufferSize;
        const size_t totalCount = m_entries.size();

        if (index == totalCount) return;
        if (totalCount == 0) return;

        {
            constexpr size_t sz = sizeof(T_Entry);
            size_t updateIndex = index;

            // NOTE KI *reallocate* SSBO if needed
            if (m_vbo.m_size < totalCount * sz) {
                m_vbo.resizeBuffer(m_entries.capacity() * sz);
                glVertexArrayVertexBuffer(vao, m_binding, m_vbo, 0, sz);
                updateIndex = 0;
            }

            const size_t updateCount = totalCount - updateIndex;

            m_vbo.update(
                updateIndex * sz,
                updateCount * sz,
                &m_entries[updateIndex]);
        }

        m_lastBufferSize = totalCount;
    }

    template<typename T_Vertex, typename T_Entry>
    void VBO<T_Vertex, T_Entry>::clear()
    {
        m_entries.clear();
        m_lastBufferSize = 0;
    }
}
