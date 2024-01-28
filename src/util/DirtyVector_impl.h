#pragma once

#include "DirtyVector.h"

namespace util
{
    constexpr size_t DIRTY_VECTOR_BLOCK_SIZE = 100;

    template<typename T>
    DirtyVector<T>::DirtyVector()
    {
        m_entries.reserve(DIRTY_VECTOR_BLOCK_SIZE);
        m_dirtyFlags.reserve(DIRTY_VECTOR_BLOCK_SIZE);
    }

    template<typename T>
    void DirtyVector<T>::reserve(size_t requestedSize)
    {
        // NOTE KI reserve but don't fill extra space always
        {
            auto count = requestedSize - m_entries.size();
            size_t size = m_entries.size() + std::max(DIRTY_VECTOR_BLOCK_SIZE, count) + DIRTY_VECTOR_BLOCK_SIZE;

            m_entries.reserve(size);
            m_dirtyFlags.reserve(size);
        }

        auto diff = requestedSize - m_entries.size();

        while (diff-- > 0) {
            m_entries.emplace_back();
            m_dirtyFlags.emplace_back(false);
        }
    }
}