#pragma once

#include "Pool.h"

namespace {
    constexpr size_t MAX_POOL_SIZE{ 100000 };
}

namespace pool {
    template<typename T>
    Pool<T>::Pool()
    {
        m_pool.reserve(MAX_POOL_SIZE);
    }

    template<typename T>
    Entry<T>& Pool<T>::getEntry(uint32_t handleIndex) noexcept
    {
        return m_pool[handleIndex];
    }

    template<typename T>
    uint32_t Pool<T>::allocate() noexcept
    {
        auto& entry = m_pool.emplace_back();
        return static_cast<uint32_t>(m_pool.size() - 1);
    }
}
