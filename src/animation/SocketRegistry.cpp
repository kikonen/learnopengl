#include "SocketRegistry.h"

#include "util/thread.h"

#include "asset/Assets.h"

#include "SocketTransformSSBO.h"

namespace {
    constexpr int INITIAL_SIZE = 10000;

    const glm::mat4 ID_MAT{ 1.f };
}

namespace animation
{
    SocketRegistry::SocketRegistry()
    {
    }

    SocketRegistry::~SocketRegistry() = default;

    void SocketRegistry::clear()
    {
        ASSERT_RT();

        m_updateReady = false;

        m_transforms.clear();
        m_slotAllocator.clear();
        m_dirtySlots.clear();

        m_snapshot.clear();
        m_dirtySnapshot.clear();

        m_transforms.reserve(INITIAL_SIZE);
        m_slotAllocator.reserve(INITIAL_SIZE);
        m_dirtySlots.reserve(INITIAL_SIZE);

        m_snapshot.reserve(INITIAL_SIZE);
        m_dirtySnapshot.reserve(INITIAL_SIZE);

        // NOTE KI null entry
        allocate(1);
    }

    void SocketRegistry::prepare()
    {
        ASSERT_RT();

        const auto& assets = Assets::get();

        clear();
    }

    util::BufferReference SocketRegistry::allocate(size_t count)
    {
        //ASSERT_WT();

        if (count == 0) return {};

        uint32_t offset;
        {
            std::lock_guard lock(m_lock);

            int32_t freeOffset = m_slotAllocator.tryAllocate(static_cast<uint32_t>(count));
            if (freeOffset >= 0) {
                offset = static_cast<uint32_t>(freeOffset);
            }
            else {
                offset = static_cast<uint32_t>(m_transforms.size());
                m_transforms.resize(m_transforms.size() + count);
                m_slotAllocator.confirmAllocation(offset, static_cast<uint32_t>(count));
            }

            for (size_t i = 0; i < count; i++) {
                m_transforms[offset + i] = ID_MAT;
            }

            markDirty({ offset, count });
        }

        return { offset, count };
    }

    util::BufferReference SocketRegistry::release(
        util::BufferReference ref)
    {
        ASSERT_WT();

        std::lock_guard lock(m_lock);

        if (!m_slotAllocator.release(ref)) return {};

        return {};
    }

    std::span<const glm::mat4> SocketRegistry::getRange(
        const util::BufferReference ref) const noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return std::span<glm::mat4>{};

        return std::span{ m_transforms }.subspan(ref.offset, ref.size);
    }

    std::span<glm::mat4> SocketRegistry::modifyRange(
        util::BufferReference ref) noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (!ref.offset) return std::span<glm::mat4>{};

        return std::span{ m_transforms }.subspan(ref.offset, ref.size);
    }

    void SocketRegistry::markDirtyAll() noexcept
    {
        m_dirtySlots.clear();
        for (const auto& [ref, allocated] : m_slotAllocator.getAllocatedSlots()) {
            if (!allocated) continue;
            m_dirtySlots.markDirty(ref);
        }
    }

    void SocketRegistry::markDirty(
        util::BufferReference ref) noexcept
    {
        m_dirtySlots.markDirty(ref);
    }

    uint32_t SocketRegistry::getActiveCount() const noexcept
    {
        return static_cast<uint32_t>(m_snapshot.size());
    }

    glm::mat4 SocketRegistry::getTransform(
        uint32_t index) const noexcept
    {
        if (index <= 0 || index >= m_transforms.size()) return glm::mat4{ 1.f };
        return m_transforms[index];
    }

    void SocketRegistry::updateWT()
    {
        makeSnapshot();
    }

    void SocketRegistry::makeSnapshot()
    {
        std::lock_guard lock(m_lock);

        m_dirtySlots.processAndClear(m_dirtySnapshot, [this](const util::BufferReference& ref) {
            const auto baseIndex = ref.offset;
            const auto updateCount = ref.size;

            const size_t totalCount = m_transforms.size();
            if (m_snapshot.size() != totalCount) {
                m_snapshot.resize(totalCount);
            }

            for (size_t i = 0; i < updateCount; i++) {
                m_snapshot[baseIndex + i] = m_transforms[baseIndex + i];
            }
        });

        if (!m_dirtySnapshot.empty()) {
            m_updateReady = true;
        }
    }
}
