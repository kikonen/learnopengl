#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>

namespace util
{
    // Thread-safe dirty slot tracker using map for automatic deduplication.
    // Uses generation counter for O(1) clear - avoids memory churn when
    // same keys are marked dirty frame after frame.
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
            m_currentGen++;
            m_dirtyCount = 0;
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
            auto& gen = m_dirty[key];
            if (gen != m_currentGen) {
                gen = m_currentGen;
                m_dirtyCount++;
            }
        }

        bool empty() const noexcept
        {
            std::lock_guard lock(m_lock);
            return m_dirtyCount == 0;
        }

        // Process dirty keys with callback and clear the dirty set.
        // Populates outSnapshot with processed keys for later use by Buffer classes.
        // Thread-safe: acquires lock during processing.
        template<typename Func>
        void processAndClear(std::vector<Key>& outSnapshot, Func&& func)
        {
            std::lock_guard lock(m_lock);

            for (const auto& [key, gen] : m_dirty) {
                if (gen != m_currentGen) continue;
                func(key);
                outSnapshot.push_back(key);
            }

            m_currentGen++;
            m_dirtyCount = 0;
        }

    private:
        mutable std::mutex m_lock{};
        uint32_t m_currentGen{ 0 };
        uint32_t m_dirtyCount{ 0 };
        std::unordered_map<Key, uint32_t> m_dirty;
    };
}
