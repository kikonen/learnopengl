#include "SnapshotRegistry.h"

#include <iostream>

#include "util/DirtyVector_impl.h"

template<typename T>
SnapshotRegistry<T>::SnapshotRegistry()
    : m_snapshots{ std::make_unique<util::DirtyVector<T>>() },
    m_pendingSnapshots{ std::make_unique<util::DirtyVector<T>>() },
    m_activeSnapshots{ std::make_unique<util::DirtyVector<T>>() }
{
    // null entry
    registerSnapshot();
    copyToPending(0, -1);
    copyFromPending(0, -1);
}

template<typename T>
SnapshotRegistry<T>::~SnapshotRegistry() = default;

template<typename T>
uint32_t SnapshotRegistry<T>::registerSnapshot() noexcept {
    return registerSnapshotRange(1);
}

template<typename T>
uint32_t SnapshotRegistry<T>::registerSnapshotRange(size_t count) noexcept {
    const uint32_t index = static_cast<uint32_t>(m_snapshots->size());

    m_snapshots->allocate(m_snapshots->size() + count);

    return index;
}

template<typename T>
void SnapshotRegistry<T>::markDirty(uint32_t index) noexcept {
    auto& dirtyFlags = m_snapshots->m_dirty;
    dirtyFlags[index] = true;
}

template<typename T>
void SnapshotRegistry<T>::clearDirty(uint32_t index) noexcept {
    auto& dirtyFlags = m_snapshots->m_dirty;
    dirtyFlags[index] = false;
}

template<typename T>
void SnapshotRegistry<T>::clearActiveDirty(uint32_t index) noexcept {
    auto& dirtyFlags = m_activeSnapshots->m_dirty;
    dirtyFlags[index] = false;
}

// NOTE KI *SWAP* approach did not work, since it caused that updates
// were blocked due to RT side holding snapshot buffer, thus it could not be updated

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

template<typename T>
void SnapshotRegistry<T>::copyToPending(uint32_t startIndex, int32_t count)
{
    std::lock_guard lock(m_lock);
    copy(*m_snapshots.get(), *m_pendingSnapshots.get(), startIndex, count);
}

template<typename T>
void SnapshotRegistry<T>::copyFromPending(uint32_t startIndex, int32_t count)
{
    std::lock_guard lock(m_lock);
    copy(*m_pendingSnapshots.get(), *m_activeSnapshots.get(), startIndex, count);
}

template<typename T>
void SnapshotRegistry<T>::copy(
    util::DirtyVector<T>& srcVector,
    util::DirtyVector<T>& dstVector,
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
                dst[i].applyFrom(src[i]);
            }
        }
    }
}

