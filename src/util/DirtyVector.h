#pragma once

namespace util
{
    template<typename T>
    struct DirtyVector {
        std::vector<T> m_entries;

        std::vector<bool> m_dirty;

        DirtyVector();
        ~DirtyVector();

        size_t size() const noexcept { return m_entries.size(); }

        void allocate(size_t size);
    };
}
