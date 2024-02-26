#pragma once

#include "DirtyVector.h"

namespace util
{
    constexpr size_t DIRTY_VECTOR_BLOCK_SIZE = 100;

    template<typename T>
    DirtyVector<T>::DirtyVector()
    {
        m_entries.reserve(DIRTY_VECTOR_BLOCK_SIZE);
        m_dirty.reserve(DIRTY_VECTOR_BLOCK_SIZE);
    }

    template<typename T>
    DirtyVector<T>::~DirtyVector() = default;

    template<typename T>
    void DirtyVector<T>::allocate(size_t requestedSize)
    {
        // NOTE KI reserve but don't fill extra space always
        {
            auto count = requestedSize - m_entries.size();
            size_t size = m_entries.size() + std::max(DIRTY_VECTOR_BLOCK_SIZE, count) + DIRTY_VECTOR_BLOCK_SIZE;

            m_entries.resize(size);
            m_dirty.resize(size);
        }
    }
}
