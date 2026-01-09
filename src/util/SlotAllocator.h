#pragma once

#include <vector>
#include <unordered_map>

#include "util/BufferReference.h"

namespace util
{
    // Manages slot allocation and deallocation for buffer-based registries.
    // Tracks allocated slots and maintains free lists by size for efficient reuse.
    // Used by JointRegistry, SocketRegistry, and similar registries.
    class SlotAllocator {
    public:
        SlotAllocator() = default;
        ~SlotAllocator() = default;

        SlotAllocator(const SlotAllocator&) = delete;
        SlotAllocator& operator=(const SlotAllocator&) = delete;

        void clear() noexcept
        {
            m_allocatedSlots.clear();
            m_freeSlots.clear();
        }

        void reserve(size_t count)
        {
            m_allocatedSlots.reserve(count);
            m_freeSlots.reserve(count);
        }

        // Try to allocate a slot of given size.
        // Returns offset if a free slot is available, or -1 if buffer needs to grow.
        // Caller should grow buffer and call confirmAllocation with the new offset.
        int32_t tryAllocate(uint32_t size) noexcept
        {
            if (size == 0) return -1;

            auto it = m_freeSlots.find(size);
            if (it != m_freeSlots.end() && !it->second.empty()) {
                uint32_t offset = it->second.back();
                it->second.pop_back();
                m_allocatedSlots[{ offset, size }] = true;
                return static_cast<int32_t>(offset);
            }

            return -1;
        }

        // Confirm allocation at given offset after growing buffer.
        void confirmAllocation(uint32_t offset, uint32_t size) noexcept
        {
            if (size == 0) return;
            m_allocatedSlots[{ offset, size }] = true;
        }

        // Release a slot back to the free list.
        // Returns true if slot was valid and released, false otherwise.
        bool release(BufferReference ref) noexcept
        {
            if (ref.size == 0) return false;
            if (ref.offset == 0) return false;  // null slot not allowed

            auto it = m_allocatedSlots.find(ref);
            if (it == m_allocatedSlots.end()) return false;

            m_freeSlots[ref.size].push_back(ref.offset);
            m_allocatedSlots[ref] = false;
            return true;
        }

        // Check if a slot is currently allocated.
        bool isAllocated(BufferReference ref) const noexcept
        {
            auto it = m_allocatedSlots.find(ref);
            return it != m_allocatedSlots.end() && it->second;
        }

        // Get all allocated slots for iteration (e.g., markDirtyAll).
        const std::unordered_map<BufferReference, bool>& getAllocatedSlots() const noexcept
        {
            return m_allocatedSlots;
        }

    private:
        std::unordered_map<BufferReference, bool> m_allocatedSlots;
        // { size: [offset, ...] }
        std::unordered_map<uint32_t, std::vector<uint32_t>> m_freeSlots;
    };
}
