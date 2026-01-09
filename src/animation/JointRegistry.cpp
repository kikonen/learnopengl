#include "JointRegistry.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "JointTransformSSBO.h"

namespace {
    constexpr int INITIAL_SIZE = 10000;

    const glm::mat4 ID_MAT{ 1.f };
}

namespace animation
{
    JointRegistry::JointRegistry()
    {
    }

    JointRegistry::~JointRegistry() = default;

    void JointRegistry::clear()
    {
        ASSERT_RT();

        m_updateReady = false;

        m_transforms.clear();
        m_freeSlots.clear();
        m_dirtySlots.clear();

        m_snapshot.clear();
        m_dirtySnapshot.clear();

        m_transforms.reserve(INITIAL_SIZE);
        m_freeSlots.reserve(INITIAL_SIZE);
        m_dirtySlots.reserve(INITIAL_SIZE);

        m_snapshot.reserve(INITIAL_SIZE);
        m_dirtySnapshot.reserve(INITIAL_SIZE);

        // NOTE KI null entry
        allocate(1);
    }

    void JointRegistry::prepare()
    {
        ASSERT_RT();

        clear();
    }

    util::BufferReference JointRegistry::allocate(size_t count)
    {
        //ASSERT_WT();

        if (count == 0) return {};

        size_t offset;
        {
            std::lock_guard lock(m_lock);

            auto it = m_freeSlots.find(static_cast<uint32_t>(count));
            if (it != m_freeSlots.end() && !it->second.empty()) {
                offset = it->second[it->second.size() - 1];
                it->second.pop_back();
            }
            else {
                offset = m_transforms.size();
                m_transforms.resize(m_transforms.size() + count);
            }

            for (int i = 0; i < count; i++) {
                m_transforms[offset + i] = ID_MAT;
            }

            m_allocatedSlots[{ offset, count }] = true;
            markDirty({ offset, count });
        }

        return { offset, count };
    }

    util::BufferReference JointRegistry::release(
        util::BufferReference ref)
    {
        ASSERT_WT();

        if (ref.size == 0) return {};

        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return {};

        std::lock_guard lock(m_lock);

        {
            auto it = m_allocatedSlots.find(ref);
            if (it == m_allocatedSlots.end()) return {};
        }

        m_freeSlots[ref.size].push_back(ref.offset);
        m_allocatedSlots[ref] = false;

        return {};
    }

    std::span<const glm::mat4> JointRegistry::getRange(
        const util::BufferReference ref) const noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return std::span<glm::mat4>{};

        return std::span{ m_transforms }.subspan(ref.offset, ref.size);
    }

    std::span<glm::mat4> JointRegistry::modifyRange(
        util::BufferReference ref) noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return std::span<glm::mat4>{};

        return std::span{ m_transforms }.subspan(ref.offset, ref.size);
    }

    void JointRegistry::markDirtyAll() noexcept
    {
        std::lock_guard lock(m_lockDirty);
        m_dirtySlots.clear();
        for (const auto& [ref, allocated] : m_allocatedSlots) {
            if (!allocated) continue;
            markDirty(ref);
        }
    }

    void JointRegistry::markDirty(
        util::BufferReference ref) noexcept
    {
        if (ref.size == 0) return;

        std::lock_guard lock(m_lockDirty);

        m_dirtySlots[ref] = true;
    }

    uint32_t JointRegistry::getActiveCount() const noexcept
    {
        return static_cast<uint32_t>(m_snapshot.size());
    }

    void JointRegistry::updateWT()
    {
        makeSnapshot();
    }

    void JointRegistry::makeSnapshot()
    {
        std::lock_guard lock(m_lock);
        std::lock_guard lockDirty(m_lockDirty);

        if (m_dirtySlots.empty()) return;

        for (const auto& [ref, dirty] : m_dirtySlots) {
            if (!dirty) continue;

            const auto baseIndex = ref.offset;
            const auto updateCount = ref.size;

            const size_t totalCount = m_transforms.size();
            if (m_snapshot.size() != totalCount) {
                m_snapshot.resize(totalCount);
            }

            for (size_t i = 0; i < updateCount; i++) {
                m_snapshot[baseIndex + i] = m_transforms[baseIndex + i];
            }

            m_dirtySnapshot.emplace_back(baseIndex, updateCount);
        }

        m_dirtySlots.clear();
        m_updateReady = true;
    }
}
