#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>

namespace util
{
    // Thread-safe dirty slot tracker using map for automatic deduplication.
    // Used by JointRegistry, SocketRegistry, and similar registries that need
    // to track dirty ranges for snapshot updates.
    template<typename Key>
    class DirtySet {
    public:
        DirtySet() = default;
        ~DirtySet() = default;

        DirtySet(const DirtySet&) = delete;
        DirtySet& operator=(const DirtySet&) = delete;

        void clear() noexcept
        {
            std::lock_guard lock(m_lock);
            m_dirty.clear();
        }

        void reserve(size_t count)
        {
            std::lock_guard lock(m_lock);
            m_dirty.reserve(count);
        }

        void markDirty(const Key& key) noexcept
        {
            if constexpr (requires { key.empty(); }) {
                if (key.empty()) return;
            }

            std::lock_guard lock(m_lock);
            m_dirty[key] = true;
        }

        bool empty() const noexcept
        {
            std::lock_guard lock(m_lock);
            return m_dirty.empty();
        }

        // Process dirty keys with callback and clear the dirty set.
        // Populates outSnapshot with processed keys for later use by Buffer classes.
        // Thread-safe: acquires lock during processing.
        template<typename Func>
        void processAndClear(std::vector<Key>& outSnapshot, Func&& func)
        {
            std::lock_guard lock(m_lock);

            if (m_dirty.empty()) return;

            for (const auto& [key, dirty] : m_dirty) {
                if (!dirty) continue;
                func(key);
                outSnapshot.push_back(key);
            }

            m_dirty.clear();
        }

    private:
        mutable std::mutex m_lock{};
        std::unordered_map<Key, bool> m_dirty;
    };
}
