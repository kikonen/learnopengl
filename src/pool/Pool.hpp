#pragma once

#include "Pool.h"

namespace {
}

namespace pool {
    template<typename T>
    Pool<T>::Pool(uint32_t size)
    {
        m_pool.reserve(size);
    }

    template<typename T>
    Entry<T>& Pool<T>::getEntry(uint32_t handleIndex) noexcept
    {
        return m_pool[handleIndex];
    }

    template<typename T>
    void Pool<T>::reserve(uint32_t size) noexcept
    {
        m_pool.reserve(size);
    }

    template<typename T>
    uint32_t Pool<T>::allocate() noexcept
    {
        auto& entry = m_pool.emplace_back();
        return static_cast<uint32_t>(m_pool.size() - 1);
    }
}
