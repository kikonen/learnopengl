#pragma once

#include "Pool.h"

namespace {
}

namespace pool {
    template<typename T>
    Pool<T>::Pool(uint32_t size)
    {
        m_pool.reserve(size);

        // NOTE KI NULL object
        m_pool.emplace_back();
    }

    template<typename T>
    Entry<T>& Pool<T>::getEntry(uint32_t handleIndex) noexcept
    {
        // NOTE KI in theory lock needed, but logic quarantees that free operations
        // are not done in unsafe locationss
        //std::shared_lock lock(m_lock);
        return m_pool[handleIndex];
    }

    template<typename T>
    void Pool<T>::reserve(uint32_t size) noexcept
    {
        std::unique_lock lock(m_lock);

        m_pool.reserve(size);
    }

    template<typename T>
    uint32_t Pool<T>::allocate() noexcept
    {
        std::unique_lock lock(m_lock);

        auto& entry = m_pool.emplace_back();
        // NOTE KI -1 since NULL object used
        return static_cast<uint32_t>(m_pool.size() - 1);
    }

    template<typename T>
    void Pool<T>::clear() noexcept
    {
        std::unique_lock lock(m_lock);

        m_pool.clear();

        //// NOTE KI NULL object
        //m_pool.emplace_back();
    }
}
