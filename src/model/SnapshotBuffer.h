#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <vector>

#include "ki/size.h"

#include "model/Snapshot.h"
#include "model/NodeState.h"

namespace model
{
    //
    // Triple-buffered snapshot storage for lock-free WT->RT communication.
    //
    // Each thread has its own private buffer, and they exchange through
    // an atomic shared buffer. This ensures WT never writes to a buffer
    // RT is reading.
    //
    // Buffer ownership:
    // - WT owns m_wtBuffer (writes here)
    // - RT owns m_rtBuffer (reads here)
    // - m_sharedIndex points to the "mailbox" buffer for exchange
    //
    class SnapshotBuffer
    {
        std::array<std::vector<Snapshot>, 3> m_buffers;

        // Shared buffer index - used for atomic exchange between WT and RT
        std::atomic<int> m_sharedIndex{ 0 };

        // Sequence number to detect new publishes (WT increments, RT reads)
        std::atomic<uint64_t> m_publishSeq{ 0 };

        // WT's private buffer index (only WT thread accesses)
        int m_wtBuffer{ 1 };

        // RT's private buffer index (only RT thread accesses)
        int m_rtBuffer{ 2 };

        // RT's last seen sequence (only RT accesses)
        uint64_t m_rtSeq{ 0 };

    public:
        void clear()
        {
            for (auto& buf : m_buffers) {
                buf.clear();
            }
            m_sharedIndex.store(0, std::memory_order_release);
            m_publishSeq.store(0, std::memory_order_release);
            m_wtBuffer = 1;
            m_rtBuffer = 2;
            m_rtSeq = 0;
        }

        void reserve(size_t size)
        {
            for (auto& buf : m_buffers) {
                buf.reserve(size);
            }
        }

        // Called by WT to publish new snapshot data
        void publish(
            const std::vector<NodeState>& states,
            const std::vector<uint32_t>& parentIndices)
        {
            const bool firstPublish = m_publishSeq.load(std::memory_order_acquire) == 0;

            auto& writeBuf = m_buffers[m_wtBuffer];

            // Copy from shared buffer first to ensure complete data
            // (shared buffer has the most recent published state)
            const int sharedIdx = m_sharedIndex.load(std::memory_order_acquire);
            const auto& sharedBuf = m_buffers[sharedIdx];

            const auto sz = states.size();
            writeBuf.resize(sz);

            // Copy all existing snapshots from shared buffer to write buffer
            const auto copySz = std::min(sz, sharedBuf.size());
            std::copy(sharedBuf.begin(), sharedBuf.begin() + copySz, writeBuf.begin());

            // Now apply updates from current state
            for (size_t i = 0; i < sz; i++) {
                // Skip free slots (0,1 are NULL/ID reserved)
                if (parentIndices[i] == 0 && i > 1) {
                    continue;
                }

                const auto& state = states[i];
                // Always copy from state - the state has the authoritative data
                writeBuf[i].applyFrom(state);
            }

            // On first publish, initialize all buffers to avoid startup flicker
            if (firstPublish) {
                for (int i = 0; i < 3; i++) {
                    if (i != m_wtBuffer) {
                        m_buffers[i].resize(sz);
                        std::copy(writeBuf.begin(), writeBuf.end(), m_buffers[i].begin());
                    }
                }
            }

            // Exchange our write buffer with the shared buffer
            // Now shared has our freshly written data, we get the old shared buffer
            m_wtBuffer = m_sharedIndex.exchange(m_wtBuffer, std::memory_order_acq_rel);

            // Increment sequence to signal new data available
            m_publishSeq.fetch_add(1, std::memory_order_release);
        }

        // Called by RT to sync and get latest snapshots
        // Returns true if new data was available
        bool sync()
        {
            // Check if there's new data since last sync
            const uint64_t currentSeq = m_publishSeq.load(std::memory_order_acquire);
            if (currentSeq == m_rtSeq) {
                // No new data, keep current buffer
                return false;
            }

            // Exchange our read buffer with the shared buffer
            // Now we have the latest data, shared gets our old buffer
            m_rtBuffer = m_sharedIndex.exchange(m_rtBuffer, std::memory_order_acq_rel);
            m_rtSeq = currentSeq;
            return true;
        }

        const std::vector<Snapshot>& getSnapshots() const noexcept
        {
            return m_buffers[m_rtBuffer];
        }

        const Snapshot* getSnapshot(uint32_t entityIndex) const noexcept
        {
            const auto& snapshots = getSnapshots();
            return snapshots.size() > entityIndex ? &snapshots[entityIndex] : nullptr;
        }

        // Fallback: check multiple buffers to handle race condition
        // when event arrives before sync is called
        const Snapshot* getSnapshotLatest(uint32_t entityIndex) const noexcept
        {
            // Try RT's buffer first
            const auto& rtBuf = m_buffers[m_rtBuffer];
            if (rtBuf.size() > entityIndex) {
                return &rtBuf[entityIndex];
            }

            // Fallback: check shared buffer (might have latest published data)
            const int sharedIdx = m_sharedIndex.load(std::memory_order_acquire);
            const auto& sharedBuf = m_buffers[sharedIdx];
            if (sharedBuf.size() > entityIndex) {
                return &sharedBuf[entityIndex];
            }

            return nullptr;
        }

        bool hasSnapshot(uint32_t entityIndex) const noexcept
        {
            return getSnapshots().size() > entityIndex;
        }

        size_t size() const noexcept
        {
            return m_buffers[m_rtBuffer].size();
        }

        // Debug: get current sequence numbers
        uint64_t getPublishSeq() const noexcept { return m_publishSeq.load(std::memory_order_acquire); }
        uint64_t getRtSeq() const noexcept { return m_rtSeq; }
        int getRtBuffer() const noexcept { return m_rtBuffer; }
        int getSharedBuffer() const noexcept { return m_sharedIndex.load(std::memory_order_acquire); }
    };
}
