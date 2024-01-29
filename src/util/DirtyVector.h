#pragma once

namespace util
{
    template<typename T>
    struct DirtyVector {
        std::vector<T> m_entries;
        std::vector<bool> m_dirtyFlags;

        DirtyVector();
        ~DirtyVector();

        size_t size() const noexcept { return m_entries.size(); }

        void reserve(size_t size);
    };
}
