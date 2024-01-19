#include "SnapshotRegistry.h"

#include "util/DirtyVector.hpp"


SnapshotRegistry::SnapshotRegistry()
    : m_snapshots{ std::make_unique<util::DirtyVector<Snapshot>>() },
    m_pendingSnapshots{ std::make_unique<util::DirtyVector<Snapshot>>() },
    m_activeSnapshots{ std::make_unique<util::DirtyVector<Snapshot>>() }
{
    // null entry
    registerSnapshot();
    copyToPending(0);
    copyFromPending(0);
}

uint32_t SnapshotRegistry::registerSnapshot() noexcept {
    return registerSnapshotRange(1);
}

uint32_t SnapshotRegistry::registerSnapshotRange(size_t count) noexcept {
    const uint32_t index = static_cast<uint32_t>(m_snapshots->size());

    m_snapshots->reserve(m_snapshots->size() + count);

    return index;
}

void SnapshotRegistry::markDirty(uint32_t index) noexcept {
    auto& dirtyFlags = m_snapshots->m_dirtyFlags;
    dirtyFlags[index] = true;
}

void SnapshotRegistry::clearDirty(uint32_t index) noexcept {
    auto& dirtyFlags = m_snapshots->m_dirtyFlags;
    dirtyFlags[index] = false;
}

void SnapshotRegistry::clearActiveDirty(uint32_t index) noexcept {
    auto& dirtyFlags = m_activeSnapshots->m_dirtyFlags;
    dirtyFlags[index] = false;
}

//void SnapshotRegistry::swap() {
//    std::lock_guard<std::mutex> lock(m_lock);
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
//    std::lock_guard<std::mutex> lock(m_lock);
//    if (m_pendingSnapshots) {
//        m_activeSnapshots.swap(m_pendingSnapshots);
//    }
//}
//
//void SnapshotRegistry::unlock() {
//    std::lock_guard<std::mutex> lock(m_lock);
//    if (m_activeSnapshots) {
//        m_activeSnapshots.swap(m_pendingSnapshots);
//    }
//}

void SnapshotRegistry::copyToPending(uint32_t startIndex)
{
    std::lock_guard<std::mutex> lock(m_lock);
    copy(*m_snapshots.get(), *m_pendingSnapshots.get(), startIndex);
}

void SnapshotRegistry::copyFromPending(uint32_t startIndex)
{
    std::lock_guard<std::mutex> lock(m_lock);
    copy(*m_pendingSnapshots.get(), *m_activeSnapshots.get(), startIndex);
}

void SnapshotRegistry::copy(
    util::DirtyVector<Snapshot>& srcVector,
    util::DirtyVector<Snapshot>& dstVector,
    uint32_t startIndex)
{
    //{
    //    auto& src = srcVector.m_entries;
    //    auto& dst = dstVector.m_entries;
    //    const auto size = src.size();

    //    dstVector.reserve(size);

    //    memcpy(&dst[startIndex], &src[startIndex], (size - startIndex) * sizeof(Snapshot));

    //    //for (auto& snapshot : dst) {
    //    //    snapshot.m_dirty = true;
    //    //}
    //}
    //{
    //    auto& src = srcVector.m_dirtyFlags;
    //    auto& dst = dstVector.m_dirtyFlags;
    //    const auto size = src.size();

    //    dstVector.reserve(size);

    //    for (size_t i = startIndex; i < size; i++) {
    //        dst[i] = src[i];
    //    }
    //}


    {
        const auto size = srcVector.size();
        dstVector.reserve(size);

        auto& src = srcVector.m_entries;
        auto& dst = dstVector.m_entries;

        // NOTE KI *CANNOT* do full memcpy since other side may not
        // have processed dirty entries, and writing those down as clean would break logic
        //memcpy(&dst[startIndex], &src[startIndex], (size - startIndex) * sizeof(Snapshot));

        for (size_t i = startIndex; i < size; i++) {
            if (src[i].m_dirty) {
                dst[i] = src[i];
                //memcpy(&dst[i], &src[i], sizeof(Snapshot));
                src[i].m_dirty = false;
                src[i].m_dirtyNormal = false;
            }
        }
    }
}

