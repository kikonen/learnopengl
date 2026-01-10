#pragma once

#include <array>
#include <atomic>
#include <vector>

#include "ki/size.h"

#include "model/Snapshot.h"
#include "model/NodeState.h"

namespace model
{
    class SnapshotBuffer
    {
        struct Buffer {
            std::vector<Snapshot> snapshots;
            std::vector<ki::level_id> lastSeenLevels;
        };

        std::array<Buffer, 2> m_buffers;
        std::atomic<int> m_readIndex{ 0 };

    public:
        void clear()
        {
            m_buffers[0].snapshots.clear();
            m_buffers[0].lastSeenLevels.clear();
            m_buffers[1].snapshots.clear();
            m_buffers[1].lastSeenLevels.clear();
            m_readIndex.store(0, std::memory_order_release);
        }

        void reserve(size_t size)
        {
            m_buffers[0].snapshots.reserve(size);
            m_buffers[0].lastSeenLevels.reserve(size);
            m_buffers[1].snapshots.reserve(size);
            m_buffers[1].lastSeenLevels.reserve(size);
        }

        void publish(
            const std::vector<NodeState>& states,
            const std::vector<uint32_t>& parentIndices)
        {
            const int writeIdx = 1 - m_readIndex.load(std::memory_order_acquire);
            const int readIdx = 1 - writeIdx;
            auto& writeBuf = m_buffers[writeIdx];
            const auto& readBuf = m_buffers[readIdx];

            const auto sz = states.size();
            writeBuf.snapshots.resize(sz);
            writeBuf.lastSeenLevels.resize(sz, 0);

            // Sync lastSeenLevels from read buffer to write buffer
            // This ensures freed slots (reset to 0 in read buffer) propagate correctly
            // and prevents stale levels from blocking slot reuse
            const auto syncSz = std::min(sz, readBuf.lastSeenLevels.size());
            for (size_t i = 0; i < syncSz; i++) {
                writeBuf.lastSeenLevels[i] = readBuf.lastSeenLevels[i];
            }

            for (size_t i = 0; i < sz; i++) {
                // Skip free slots (0,1 are NULL/ID reserved)
                if (parentIndices[i] == 0 && i > 1) {
                    writeBuf.lastSeenLevels[i] = 0;
                    continue;
                }

                const auto& state = states[i];
                if (state.m_matrixLevel > writeBuf.lastSeenLevels[i]) {
                    writeBuf.snapshots[i].applyFrom(state);
                    writeBuf.lastSeenLevels[i] = state.m_matrixLevel;
                }
            }

            m_readIndex.store(writeIdx, std::memory_order_release);
        }

        const std::vector<Snapshot>& getSnapshots() const noexcept
        {
            const int readIdx = m_readIndex.load(std::memory_order_acquire);
            return m_buffers[readIdx].snapshots;
        }

        const Snapshot* getSnapshot(uint32_t entityIndex) const noexcept
        {
            const auto& snapshots = getSnapshots();
            return snapshots.size() > entityIndex ? &snapshots[entityIndex] : nullptr;
        }

        // Fallback: check both buffers to handle race condition
        // when event arrives before atomic read index is visible
        const Snapshot* getSnapshotLatest(uint32_t entityIndex) const noexcept
        {
            // Try current read buffer first
            const int readIdx = m_readIndex.load(std::memory_order_acquire);
            const auto& readBuf = m_buffers[readIdx];
            if (readBuf.snapshots.size() > entityIndex) {
                return &readBuf.snapshots[entityIndex];
            }

            // Fallback: check other buffer (might have latest published data)
            const int writeIdx = 1 - readIdx;
            const auto& writeBuf = m_buffers[writeIdx];
            return writeBuf.snapshots.size() > entityIndex ? &writeBuf.snapshots[entityIndex] : nullptr;
        }

        bool hasSnapshot(uint32_t entityIndex) const noexcept
        {
            return getSnapshots().size() > entityIndex;
        }

        size_t size() const noexcept
        {
            const int readIdx = m_readIndex.load(std::memory_order_acquire);
            return m_buffers[readIdx].snapshots.size();
        }
    };
}
