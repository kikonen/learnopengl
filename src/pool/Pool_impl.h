#pragma once

#include "Pool.h"

namespace {
    constexpr int32_t IN_USE = -1;
    constexpr int32_t TERMINATOR = -2;
}

namespace pool {
    template<typename T>
    Pool<T>::Pool(uint32_t blockSize)
        : m_blockSize{ blockSize },
        m_entrySize{ sizeof(Entry<T>) }
    {
        m_pool = reinterpret_cast<Entry<T>*>(malloc(m_blockSize * m_entrySize));
        memset(m_pool, 0, m_blockSize);

        m_nextFree = 0;
        for (uint32_t i = 0; i < blockSize; i++) {
            auto& entry = m_pool[i];
            entry.m_nextFree = i + 1;
        }
        m_pool[blockSize - 1].m_nextFree = -2;

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

        if (m_nextFree == TERMINATOR) return 0;

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

        // NOTE KI release all used entries for clean shutdown
        for (uint32_t i = 0; i < m_blockSize; i++) {
            auto& entry = m_pool[i];
            if (entry.m_nextFree == IN_USE) {
                entry.~Entry<T>();
            }
        }
        free((void*)m_pool);

        m_pool = nullptr;
    }
}
