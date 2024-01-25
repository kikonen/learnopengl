#pragma once

#include <vector>
#include <mutex>

#include "Entry.h"

namespace pool {
    template<typename T>
    class Pool {
    public:
        Pool(uint32_t size);

        Entry<T>& getEntry(uint32_t index) noexcept;

        void reserve(uint32_t size) noexcept;
        uint32_t allocate() noexcept;

        void clear() noexcept;
    private:
        std::vector<Entry<T>> m_pool;
        std::mutex m_lock{};
    };
}
