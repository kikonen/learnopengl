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
        m_entries.resize(requestedSize);
        m_dirty.resize(requestedSize);
    }
}
