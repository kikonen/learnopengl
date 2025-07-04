#pragma once

#include "Pool.h"

namespace {
}

namespace pool {
    inline int32_t NULL_INDEX = 0;
    inline int32_t IN_USE = -1;
    inline int32_t TERMINATOR = -2;

    template<typename T>
    Pool<T>::Pool(uint32_t blockSize)
        : m_blockSize{ blockSize },
        m_entrySize{ sizeof(Entry<T>) }
    {
        m_pool = reinterpret_cast<Entry<T>*>(malloc(m_blockSize * m_entrySize));
        memset(m_pool, 0, m_blockSize);

        clear(false);
    }

    template<typename T>
    void Pool<T>::destroy() noexcept
    {
        clear(true);
    }

    template<typename T>
    void Pool<T>::clear(bool destroy) noexcept
    {
        {
            std::lock_guard lock(m_lock);

            if (!m_pool) return;

            {
                // NOTE KI release all used entries for clean shutdown
                for (uint32_t i = 0; i < m_blockSize; i++) {
                    auto& entry = m_pool[i];
                    if (entry.m_nextFree == IN_USE) {
                        entry.~Entry<T>();
                    }
                }
            }

            m_nextFree = 0;
            for (uint32_t i = 0; i < m_blockSize; i++) {
                auto& entry = m_pool[i];
                entry.m_nextFree = i + 1;
            }
            m_pool[m_blockSize - 1].m_nextFree = -2;

            if (destroy) {
                free((void*)m_pool);
                m_pool = nullptr;
            }
        }

        if (!destroy) {
            // NOTE KI NULL object
            allocate();
        }
    }

    template<typename T>
    Entry<T>* Pool<T>::getEntry(uint32_t handleIndex) noexcept
    {
        // NOTE KI in theory lock needed, but logic quarantees that free operations
        // are not done in unsafe locationss
        //std::shared_lock lock(m_lock);
        return m_pool ? &m_pool[handleIndex] : nullptr;
    }

    template<typename T>
    void Pool<T>::release(uint32_t index) noexcept
    {
        std::lock_guard lock(m_lock);

        if (!m_pool) return;

        // TODO KI release memory properly
        // https://stackoverflow.com/questions/13210757/how-to-call-destructor-of-type-in-template
        auto& entry = m_pool[index];
        entry.~Entry();

        entry.m_nextFree = m_nextFree;
        m_nextFree = index;
    }

    template<typename T>
    uint32_t Pool<T>::allocate() noexcept
    {
        std::lock_guard lock(m_lock);

        if (m_nextFree == TERMINATOR) return NULL_INDEX;
        if (!m_pool) return NULL_INDEX;

        uint32_t index = m_nextFree;

        auto& entry = m_pool[index];
        int32_t nextFree = entry.m_nextFree;

        // TODO KI allocate memory properly
        new(&entry) Entry<T>{};

        m_nextFree = nextFree;

        return index;
    }
}
