#pragma once

namespace pool {
    template<typename T>
    struct Entry
    {
        Entry() {}
        Entry(Entry& o) = delete;
        Entry(const Entry&) = delete;
        Entry(Entry&& o) noexcept
        : m_data { std::move(o.m_data) }
        {}

        ~Entry() {}

        Entry& operator=(Entry& o) = delete;
        Entry& operator=(Entry&& o) noexcept
        {
            m_data = std::move(o.data);
        }

        T m_data;
    };
}
