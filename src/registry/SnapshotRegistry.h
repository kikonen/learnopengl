#pragma once

#include <vector>
#include <span>
#include <mutex>
#include <stdint.h>

#include "util/DirtyVector.h"

#include "model/Snapshot.h"


class SnapshotRegistry final {

public:
    SnapshotRegistry();

    ~SnapshotRegistry() = default;

    //inline const Snapshot& operator[](uint32_t index) const noexcept {
    //    auto& snapshots = m_activeSnapshots->m_entries;
    //    return snapshots[index];
    //}

    inline const Snapshot& getActiveSnapshot(uint32_t index) const noexcept {
        auto& snapshots = m_activeSnapshots->m_entries;
        return snapshots[index];
    }

    inline const std::span<Snapshot> getActiveSnapshotRange(uint32_t start, uint32_t count) noexcept {
        auto& snapshots = m_activeSnapshots->m_entries;
        return std::span{ snapshots }.subspan(start, count);
    }

    inline const Snapshot& getSnapshot(uint32_t index) const noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return snapshots[index];
    }

    inline const std::span<Snapshot> getSnapshotRange(uint32_t start, uint32_t count) noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return std::span{ snapshots }.subspan(start, count);
    }

    inline Snapshot& modifySnapshot(uint32_t index) noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return snapshots[index];
    }

    inline std::span<Snapshot> modifySnapshotRange(uint32_t start, uint32_t count) noexcept {
        auto& snapshots = m_snapshots->m_entries;
        return std::span{ snapshots }.subspan(start, count);
    }

    void clearActiveDirty(uint32_t index) noexcept;

    void markDirty(uint32_t index) noexcept;
    void clearDirty(uint32_t index) noexcept;

    uint32_t registerSnapshot() noexcept;
    uint32_t registerSnapshotRange(size_t count) noexcept;

    // NOTE KI swap only if pending is NOT empty
    void swap();

    // NOTE KI swap only if active is empty
    void lock();

    // NOTE KI swap only if pending is empty
    void unlock();

private:
    std::mutex m_lock{};

    std::unique_ptr<util::DirtyVector<Snapshot>> m_snapshots;
    std::unique_ptr<util::DirtyVector<Snapshot>> m_pendingSnapshots;
    std::unique_ptr<util::DirtyVector<Snapshot>> m_activeSnapshots{ nullptr };
};
