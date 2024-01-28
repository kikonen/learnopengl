#pragma once

#include "Pool.h"

namespace {
}

namespace pool {
    template<typename T>
    Pool<T>::Pool(uint32_t blockSize)
        : m_blockSize{ blockSize },
        m_entrySize{ sizeof(Entry<T>) }
    {
        m_pool = new Entry<T>[blockSize];

        m_nextFree = 0;
        for (uint32_t i = 0; i < blockSize; i++) {
            m_pool[i].m_nextFree = i + 1;
        }
        m_pool[blockSize - 1].m_nextFree = -1;

        // NOTE KI NULL object
        allocate();
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
    void Pool<T>::release(uint32_t index) noexcept
    {
        std::unique_lock lock(m_lock);

        // TODO KI release memory properly
        auto& entry = m_pool[index];
        entry.~Entry<T>();
        entry.m_nextFree = m_nextFree;
        m_nextFree = index;
    }

    template<typename T>
    uint32_t Pool<T>::allocate() noexcept
    {
        std::unique_lock lock(m_lock);

        if (m_nextFree == -1) return 0;

        uint32_t index = m_nextFree;

        auto& entry = m_pool[index];
        int32_t nextFree = entry.m_nextFree;

        // TODO KI allocate memory properly
        new(&entry) Entry<T>{};

        m_nextFree = nextFree;

        return index;
    }

    template<typename T>
    void Pool<T>::clear() noexcept
    {
        std::unique_lock lock(m_lock);

        delete[] m_pool;
        m_pool = nullptr;
    }
}
