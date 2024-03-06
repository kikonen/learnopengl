#pragma once

#include <vector>
#include <span>
#include <mutex>
#include <stdint.h>
#include <functional>

#include "util/DirtyVector.h"

// Maintain separate snapshot copies for WT & RT
// - intermediate "pending" copy required to allow both threads
//   to do processing without constant locking of data
// - i.e. neither RT or WT can directly touch other side snapshots since
//   that would be immediate race condition
template<typename T>
class SnapshotRegistry {
public:
    SnapshotRegistry();
    ~SnapshotRegistry();

    inline T& modifySnapshot(uint32_t index) noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return snapshots[index];
    }

    inline std::span<T> modifySnapshotRange(uint32_t start, uint32_t count) noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return std::span{ snapshots }.subspan(start, count);
    }

    inline const T& getSnapshot(uint32_t index) const noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return snapshots[index];
    }

    inline std::span<const T> getSnapshotRange(uint32_t start, uint32_t count) const noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return std::span{ snapshots }.subspan(start, count);
    }

    inline bool hasSnapshot(uint32_t index) const noexcept {
        return index < m_snapshots->size();
    }

    inline bool hasSnapshotRange(uint32_t start, uint32_t count) const noexcept {
        return start + count <= m_snapshots->size();
    }

    void markDirty(uint32_t index) noexcept;
    void clearDirty(uint32_t index) noexcept;

    uint32_t registerSnapshot() noexcept;
    uint32_t registerSnapshotRange(size_t count) noexcept;

    // NOTE KI *SWAP* approach did not work, since it caused that updates
    // were blocked due to RT side holding snapshot buffer, thus it could not be updated

    void withLock(const std::function<void()>& fn)
    {
        std::lock_guard lock(m_lock);
        fn();
    }

    // lock this && copy to
    inline void copyTo(
        SnapshotRegistry<T>* dst,
        uint32_t startIndex,
        int32_t count) noexcept
    {
        std::lock_guard lock(m_lock);
        copy(m_snapshots.get(), dst->m_snapshots.get(), startIndex, count);
    }

    // lock this && copy from
    inline void copyFrom(
        SnapshotRegistry<T>* src,
        uint32_t startIndex,
        int32_t count) noexcept
    {
        std::lock_guard lock(m_lock);
        copy(src->m_snapshots.get(), m_snapshots.get(), startIndex, count);
    }

protected:
    void copy(
        util::DirtyVector<T>* src,
        util::DirtyVector<T>* dst,
        uint32_t startIndex,
        int32_t count) noexcept;

    inline void apply(const T& src, T& dst) noexcept {
        dst.applyFrom(src);
    }

protected:
    mutable std::mutex m_lock;

    std::unique_ptr<util::DirtyVector<T>> m_snapshots;
};
