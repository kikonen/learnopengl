#pragma once

namespace util
{
    template<typename T>
    struct DirtyVector {
        std::vector<T> m_entries;
        std::vector<bool> m_dirtyFlags;

        size_t size() const noexcept { return m_entries.size(); }

        void reserve(size_t size)
        {
            auto diff = size - m_entries.size();
            m_entries.reserve(size);
            m_dirtyFlags.reserve(size);

            while (diff-- > 0) {
                m_entries.emplace_back();
                m_dirtyFlags.emplace_back(false);
            }
        }
    };
}
