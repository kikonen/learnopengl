#pragma once

#include <atomic>
#include <shared_mutex>

#include "Entry.h"

namespace pool {
    template<typename T>
    class Pool {
    public:
        Pool(uint32_t blockSize);

        void destroy() noexcept;
        void clear(bool destroy) noexcept;

        Entry<T>* getEntry(uint32_t index) noexcept;

        void release(uint32_t index) noexcept;
        uint32_t allocate() noexcept;

    private:
        const uint32_t m_blockSize;
        const uint32_t m_entrySize;

        int32_t m_nextFree;
        Entry<T>* m_pool;

        std::mutex m_lock;
    };
}
