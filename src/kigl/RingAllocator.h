#pragma once

#include <string>
#include <string_view>

#include "kigl/kigl.h"
#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

#include "util/BufferReference.h"

namespace kigl
{
    constexpr size_t DEFAULT_RING_FRAMES = 3;
    constexpr size_t MAX_RING_FRAMES = 4;

    template<typename T>
    struct RingAllocation

    {
        T* data;
        util::BufferReference ref;

        operator bool() const noexcept { return data != nullptr; }

        T* operator->() noexcept { return data; }
        const T* operator->() const noexcept { return data; }

        T& operator*() noexcept { return *data; }
        const T& operator*() const noexcept { return *data; }

        T& operator[](size_t index) noexcept { return data[index]; }
        const T& operator[](size_t index) const noexcept { return data[index]; }
    };

    // Explicit specialization for void - cannot have reference operators
    template<>
    struct RingAllocation<void>
    {
        void* data;
        util::BufferReference ref;

        operator bool() const noexcept { return data != nullptr; }
    };

    class RingAllocator
    {
    public:
        RingAllocator(
            std::string_view name,
            size_t alignment = 4,
            size_t ringFrames = DEFAULT_RING_FRAMES,
            float growthFactor = 1.5f);

        ~RingAllocator() = default;

        RingAllocator(const RingAllocator&) = delete;
        RingAllocator& operator=(const RingAllocator&) = delete;

        RingAllocator(RingAllocator&&) noexcept = default;
        RingAllocator& operator=(RingAllocator&&) noexcept = default;

        void create(size_t sizePerFrame, bool preserveOnResize = false);

        void beginFrame();
        void endFrame();

        // Allocate single element
        template<typename T>
        RingAllocation<T> allocate();

        // Allocate array of count elements
        template<typename T>
        RingAllocation<T> allocate(size_t count);

        // Allocate raw bytes
        RingAllocation<void> allocateBytes(size_t bytes);

        // Try variants - don't trigger resize on failure
        template<typename T>
        RingAllocation<T> tryAllocate();

        template<typename T>
        RingAllocation<T> tryAllocate(size_t count);

        RingAllocation<void> tryAllocateBytes(size_t bytes);

        void requestResize(size_t newSizePerFrame);

        // Bindings
        void bindUBO(GLuint binding, const util::BufferReference& ref);
        void bindSSBO(GLuint binding, const util::BufferReference& ref);
        void bindDrawIndirect();

        // Get pointer from reference (for deferred access)
        template<typename T>
        T* mapped(const util::BufferReference& ref);

        // Accessors
        GLuint id() const { return m_buffer.id(); }
        size_t usedThisFrame() const;
        size_t remainingThisFrame() const;
        size_t sizePerFrame() const { return m_sizePerFrame; }
        size_t totalSize() const { return m_sizePerFrame * m_ringFrames; }
        size_t alignment() const { return m_alignment; }
        size_t peakUsage() const { return m_peakUsageOverall; }
        bool hadOverflow() const { return m_overflowThisFrame; }

    private:
        static size_t alignUp(size_t value, size_t alignment);

        void createBuffer();
        void handleOverflow(size_t requiredSize);
        void performResize();

        RingAllocation<void> allocateImpl(size_t bytes, bool triggerResize);

        std::string m_name;
        GLBuffer m_buffer;

        size_t m_alignment;
        size_t m_ringFrames;
        float m_growthFactor;
        size_t m_sizePerFrame = 0;
        bool m_preserveOnResize = false;

        size_t m_currentFrame = 0;
        size_t m_frameStartOffset = 0;
        size_t m_currentOffset = 0;

        GLFence m_fences[MAX_RING_FRAMES];

        bool m_pendingResize = false;
        size_t m_requestedSize = 0;
        bool m_overflowThisFrame = false;

        size_t m_peakUsageThisFrame = 0;
        size_t m_peakUsageOverall = 0;
    };

    // Template implementations

    template<typename T>
    RingAllocation<T> RingAllocator::allocate()
    {
        auto alloc = allocateImpl(sizeof(T), true);
        return { static_cast<T*>(alloc.data), alloc.ref };
    }

    template<typename T>
    RingAllocation<T> RingAllocator::allocate(size_t count)
    {
        auto alloc = allocateImpl(count * sizeof(T), true);
        return { static_cast<T*>(alloc.data), alloc.ref };
    }

    template<typename T>
    RingAllocation<T> RingAllocator::tryAllocate()
    {
        auto alloc = allocateImpl(sizeof(T), false);
        return { static_cast<T*>(alloc.data), alloc.ref };
    }

    template<typename T>
    RingAllocation<T> RingAllocator::tryAllocate(size_t count)
    {
        auto alloc = allocateImpl(count * sizeof(T), false);
        return { static_cast<T*>(alloc.data), alloc.ref };
    }

    template<typename T>
    T* RingAllocator::mapped(const util::BufferReference& ref)
    {
        return m_buffer.mapped<T>(ref.offset);
    }

}
