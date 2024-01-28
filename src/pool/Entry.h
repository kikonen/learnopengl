#pragma once

#include <stdint.h>

namespace pool {
    template<typename T>
    struct Entry
    {
        Entry() {}
        Entry(Entry& o) = delete;
        Entry(const Entry&) = delete;
        Entry(Entry&& o) noexcept
            : m_nextFree{ o.m_nextFree },
            m_data { std::move(o.m_data) }
        {}

        ~Entry() {}

        Entry& operator=(Entry& o) = delete;
        Entry& operator=(Entry&& o) noexcept
        {
            m_nextFree = o.m_nextFree;
            m_data = std::move(o.data);
        }

        int32_t m_nextFree{ -1 };

        T m_data;
    };
}
