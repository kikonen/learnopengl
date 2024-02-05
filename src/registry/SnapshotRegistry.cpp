#include "SnapshotRegistry.h"

#include <iostream>

#include "util/DirtyVector_impl.h"


SnapshotRegistry::SnapshotRegistry()
    : m_snapshots{ std::make_unique<util::DirtyVector<Snapshot>>() },
    m_pendingSnapshots{ std::make_unique<util::DirtyVector<Snapshot>>() },
    m_activeSnapshots{ std::make_unique<util::DirtyVector<Snapshot>>() }
{
    // null entry
    registerSnapshot();
    copyToPending(0, -1);
    copyFromPending(0, -1);
}

SnapshotRegistry::~SnapshotRegistry() = default;

uint32_t SnapshotRegistry::registerSnapshot() noexcept {
    return registerSnapshotRange(1);
}

uint32_t SnapshotRegistry::registerSnapshotRange(size_t count) noexcept {
    const uint32_t index = static_cast<uint32_t>(m_snapshots->size());

    m_snapshots->allocate(m_snapshots->size() + count);

    return index;
}

void SnapshotRegistry::markDirty(uint32_t index) noexcept {
    auto& dirtyFlags = m_snapshots->m_dirty;
    dirtyFlags[index] = true;
}

void SnapshotRegistry::clearDirty(uint32_t index) noexcept {
    auto& dirtyFlags = m_snapshots->m_dirty;
    dirtyFlags[index] = false;
}

void SnapshotRegistry::clearActiveDirty(uint32_t index) noexcept {
    auto& dirtyFlags = m_activeSnapshots->m_dirty;
    dirtyFlags[index] = false;
}

//void SnapshotRegistry::swap() {
//    std::lock_guard lock(m_lock);
//    // NOTE KI swap only if pending is not locked by RT
//    if (!m_pendingSnapshots) return;
//
//    {
//        auto& src = m_snapshots->m_entries;
//        auto& dst = m_pendingSnapshots->m_entries;
//        const auto size = src.size();
//
//        m_pendingSnapshots->reserve(size);
//        memcpy(dst.data(), src.data(), size * sizeof(Snapshot));
//    }
//    {
//        auto& src = m_snapshots->m_dirtyFlags;
//        auto& dst = m_pendingSnapshots->m_dirtyFlags;
//        const auto size = src.size();
//
//        m_pendingSnapshots->reserve(size);
//
//        for (size_t i = 0; i < size; i++) {
//            dst[i] = src[i];
//        }
//    }
//
//    m_pendingSnapshots.swap(m_snapshots);
//}
//
//void SnapshotRegistry::lock() {
//    std::lock_guard lock(m_lock);
//    if (m_pendingSnapshots) {
//        m_activeSnapshots.swap(m_pendingSnapshots);
//    }
//}
//
//void SnapshotRegistry::unlock() {
//    std::lock_guard lock(m_lock);
//    if (m_activeSnapshots) {
//        m_activeSnapshots.swap(m_pendingSnapshots);
//    }
//}

void SnapshotRegistry::copyToPending(uint32_t startIndex, int32_t count)
{
    std::lock_guard lock(m_lock);
    copy(*m_snapshots.get(), *m_pendingSnapshots.get(), startIndex, count);
}

void SnapshotRegistry::copyFromPending(uint32_t startIndex, int32_t count)
{
    std::lock_guard lock(m_lock);
    copy(*m_pendingSnapshots.get(), *m_activeSnapshots.get(), startIndex, count);
}

void SnapshotRegistry::copy(
    util::DirtyVector<Snapshot>& srcVector,
    util::DirtyVector<Snapshot>& dstVector,
    uint32_t startIndex,
    int32_t requestedCount)
{
    size_t count = requestedCount;
    if (requestedCount == -1) {
        count = srcVector.size() - startIndex;
    }
    if (!count) return;

    {
        auto& src = srcVector.m_entries;
        auto& dst = dstVector.m_entries;

        dstVector.allocate(srcVector.size());

        for (size_t i = startIndex; i < startIndex + count; i++) {
            if (src[i].m_dirty) {
                dst[i].apply(src[i]);
            }
        }
    }
}

