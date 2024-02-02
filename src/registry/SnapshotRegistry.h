#pragma once

#include <vector>
#include <span>
#include <mutex>
#include <stdint.h>

#include "util/DirtyVector.h"

#include "model/Snapshot.h"

// Maintain separate snapshot copies for WT & RT
// - intermediate "pending" copy required to allow both threads
//   to do processing without constant locking of data
// - i.e. neither RT or WT can directly touch other side snapshots since
//   that would be immediate race condition
class SnapshotRegistry final {
public:
    SnapshotRegistry();
    ~SnapshotRegistry();

    inline Snapshot& modifySnapshot(uint32_t index) noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return snapshots[index];
    }

    inline std::span<Snapshot> modifySnapshotRange(uint32_t start, uint32_t count) noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return std::span{ snapshots }.subspan(start, count);
    }

    inline bool hasActiveSnapshot(uint32_t index) const noexcept {
        return index < m_activeSnapshots->size();
    }

    inline const Snapshot& getActiveSnapshot(uint32_t index) const noexcept {
        auto& snapshots = m_activeSnapshots->m_entries;
        return snapshots[index];
    }

    inline bool hasActiveSnapshotRange(uint32_t start, uint32_t count) const noexcept {
        return start + count <= m_activeSnapshots->size();
    }

    inline const std::span<const Snapshot> getActiveSnapshotRange(uint32_t start, uint32_t count) const noexcept {
        const auto& snapshots = m_activeSnapshots->m_entries;
        return std::span{ snapshots }.subspan(start, count);
    }

    inline Snapshot& modifyActiveSnapshot(uint32_t index) noexcept {
        auto& snapshots = m_activeSnapshots->m_entries;
        return snapshots[index];
    }

    inline std::span<Snapshot> modifyActiveSnapshotRange(uint32_t start, uint32_t count) noexcept {
        auto& snapshots = m_activeSnapshots->m_entries;
        return std::span{ snapshots }.subspan(start, count);
    }


    void clearActiveDirty(uint32_t index) noexcept;

    void markDirty(uint32_t index) noexcept;
    void clearDirty(uint32_t index) noexcept;

    uint32_t registerSnapshot() noexcept;
    uint32_t registerSnapshotRange(size_t count) noexcept;

    //// NOTE KI swap only if pending is NOT empty
    //void swap();

    //// NOTE KI swap only if active is empty
    //void lock();

    //// NOTE KI swap only if pending is empty
    //void unlock();

    void copyToPending(uint32_t startIndex, int32_t count);
    void copyFromPending(uint32_t startIndex, int32_t count);

private:
    void copy(
        util::DirtyVector<Snapshot>& src,
        util::DirtyVector<Snapshot>& dst,
        uint32_t startIndex,
        int32_t count);

private:
    std::mutex m_lock;

    std::unique_ptr<util::DirtyVector<Snapshot>> m_snapshots;
    std::unique_ptr<util::DirtyVector<Snapshot>> m_pendingSnapshots;
    std::unique_ptr<util::DirtyVector<Snapshot>> m_activeSnapshots{ nullptr };

    std::vector<bool> m_dirtyNormalTmp;
};
