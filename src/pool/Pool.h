#pragma once

#include <vector>

#include "Entry.h"

namespace pool {
    template<typename T>
    class Pool {
    public:
        Pool();

        Entry<T>& getEntry(uint32_t index) noexcept;

        uint32_t allocate() noexcept;

    private:
        std::vector<Entry<T>> m_pool;
    };
}
