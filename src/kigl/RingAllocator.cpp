#include "RingAllocator.h"

#include <cassert>
#include <algorithm>

#include <fmt/format.h>

namespace kigl
{

    RingAllocator::RingAllocator(
        std::string_view name,
        size_t alignment,
        size_t ringFrames,
        float growthFactor)
        : m_name(name),
        m_buffer(name),
        m_alignment(alignment),
        m_ringFrames(ringFrames),
        m_growthFactor(growthFactor)
    {
        assert(ringFrames >= 2 && ringFrames <= MAX_RING_FRAMES);
        assert((alignment & (alignment - 1)) == 0);
        assert(growthFactor > 1.0f);

        for (size_t i = 0; i < MAX_RING_FRAMES; i++) {
            m_fences[i] = GLFence(fmt::format("{}_fence_{}", name, i));
        }
    }

    void RingAllocator::create(size_t sizePerFrame, bool preserveOnResize)
    {
        m_sizePerFrame = sizePerFrame;
        m_preserveOnResize = preserveOnResize;
        createBuffer();
    }

    void RingAllocator::createBuffer()
    {
        size_t totalSize = m_sizePerFrame * m_ringFrames;
        GLuint flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        m_buffer.createEmpty(totalSize, flags);
        m_buffer.map(flags);
    }

    void RingAllocator::beginFrame()
    {
        size_t frameIndex = m_currentFrame % m_ringFrames;

        m_fences[frameIndex].waitFence();

        if (m_pendingResize) {
            performResize();
            m_pendingResize = false;
        }

        m_frameStartOffset = frameIndex * m_sizePerFrame;
        m_currentOffset = m_frameStartOffset;
        m_peakUsageThisFrame = 0;
        m_overflowThisFrame = false;
    }

    void RingAllocator::endFrame()
    {
        size_t frameIndex = m_currentFrame % m_ringFrames;

        m_fences[frameIndex].setFence();

        m_peakUsageOverall = std::max(m_peakUsageOverall, m_peakUsageThisFrame);

        m_currentFrame++;
    }

    RingAllocation<void> RingAllocator::allocateImpl(size_t bytes, bool triggerResize)
    {
        size_t aligned = alignUp(m_currentOffset, m_alignment);
        size_t frameEnd = m_frameStartOffset + m_sizePerFrame;

        if (aligned + bytes > frameEnd) {
            if (triggerResize) {
                handleOverflow(aligned + bytes - m_frameStartOffset);
            }
            return { nullptr, {} };
        }

        m_currentOffset = aligned + bytes;
        m_peakUsageThisFrame = std::max(m_peakUsageThisFrame, m_currentOffset - m_frameStartOffset);

        return { m_buffer.mapped<void>(aligned), { aligned, bytes } };
    }

    RingAllocation<void> RingAllocator::allocateBytes(size_t bytes)
    {
        return allocateImpl(bytes, true);
    }

    RingAllocation<void> RingAllocator::tryAllocateBytes(size_t bytes)
    {
        return allocateImpl(bytes, false);
    }

    void RingAllocator::requestResize(size_t newSizePerFrame)
    {
        if (newSizePerFrame > m_sizePerFrame) {
            m_requestedSize = newSizePerFrame;
            m_pendingResize = true;
        }
    }

    void RingAllocator::handleOverflow(size_t requiredSize)
    {
        m_overflowThisFrame = true;

        size_t newSize = m_sizePerFrame;
        while (newSize < requiredSize) {
            newSize = static_cast<size_t>(newSize * m_growthFactor);
        }

        newSize = alignUp(newSize, 64 * 1024);

        m_requestedSize = newSize;
        m_pendingResize = true;

        KI_DEBUG(fmt::format(
            "RingAllocator '{}': overflow, scheduling resize {} -> {}",
            m_name, m_sizePerFrame, newSize));
    }

    void RingAllocator::performResize()
    {
        size_t newTotalSize = m_requestedSize * m_ringFrames;

        if (m_preserveOnResize) {
            m_buffer.markUsed(m_buffer.size());
        }

        m_buffer.resizeBuffer(newTotalSize, m_preserveOnResize);

        GLuint flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        m_buffer.map(flags);

        m_sizePerFrame = m_requestedSize;

        for (size_t i = 0; i < m_ringFrames; i++) {
            m_fences[i].release();
        }

        KI_DEBUG(fmt::format(
            "RingAllocator '{}': resized to {} per frame ({} total)",
            m_name, m_sizePerFrame, totalSize()));
    }

    void RingAllocator::bindUBO(GLuint binding, const util::BufferReference& ref)
    {
        m_buffer.bindUBORange(binding, ref.offset, ref.size);
    }

    void RingAllocator::bindSSBO(GLuint binding, const util::BufferReference& ref)
    {
        m_buffer.bindSSBORange(binding, ref.offset, ref.size);
    }

    void RingAllocator::bindDrawIndirect()
    {
        m_buffer.bindDrawIndirect();
    }

    size_t RingAllocator::usedThisFrame() const
    {
        return m_currentOffset - m_frameStartOffset;
    }

    size_t RingAllocator::remainingThisFrame() const
    {
        return m_sizePerFrame - usedThisFrame();
    }

    size_t RingAllocator::alignUp(size_t value, size_t alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

}
