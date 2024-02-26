#include "SnapshotRegistry.h"

#include <iostream>

#include "util/DirtyVector_impl.h"

template<typename T>
SnapshotRegistry<T>::SnapshotRegistry()
    : m_snapshots{ std::make_unique<util::DirtyVector<T>>() }
{
    // null entry
    registerSnapshot();
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
void SnapshotRegistry<T>::copy(
    util::DirtyVector<T>* srcVector,
    util::DirtyVector<T>* dstVector,
    uint32_t startIndex,
    int32_t requestedCount) noexcept
{
    size_t count = requestedCount;
    if (requestedCount == -1) {
        count = srcVector->size() - startIndex;
    }
    if (!count) return;

    {
        const auto& src = srcVector->m_entries;
        auto& dst = dstVector->m_entries;

        dstVector->allocate(srcVector->size());

        for (size_t i = startIndex; i < startIndex + count; i++) {
            if (src[i].m_dirty) {
                apply(src[i], dst[i]);
            }
        }
    }
}

