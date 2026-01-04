#include "RigNodeRegistry.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "JointTransformSSBO.h"

namespace {
    constexpr int INITIAL_SIZE = 10000;

    const glm::mat4 ID_MAT{ 1.f };
}

namespace animation
{
    RigNodeRegistry::RigNodeRegistry()
    {
    }

    RigNodeRegistry::~RigNodeRegistry() = default;

    void RigNodeRegistry::clear()
    {
        ASSERT_RT();

        m_updateReady = false;

        m_transforms.clear();
        m_freeSlots.clear();
        m_dirtySlots.clear();

        m_transforms.reserve(INITIAL_SIZE);
        m_freeSlots.reserve(INITIAL_SIZE);
        m_dirtySlots.reserve(INITIAL_SIZE);

        // NOTE KI null entry
        allocate(1);
    }

    void RigNodeRegistry::prepare()
    {
        ASSERT_RT();

        clear();
    }

    util::BufferReference RigNodeRegistry::allocate(uint32_t count)
    {
        //ASSERT_WT();

        if (count == 0) return {};

        uint32_t offset;
        {
            std::lock_guard lock(m_lock);

            auto it = m_freeSlots.find(count);
            if (it != m_freeSlots.end() && !it->second.empty()) {
                auto& offsets = it->second;
                offset = offsets[offsets.size() - 1];
                offsets.pop_back();
            }
            else {
                offset = static_cast<uint32_t>(m_transforms.size());
                m_transforms.resize(m_transforms.size() + count);
            }

            for (unsigned int i = 0; i < count; i++) {
                m_transforms[offset + i] = ID_MAT;
            }

            markDirty({ offset, count });
        }

        return { offset, count };
    }

    util::BufferReference RigNodeRegistry::release(
        util::BufferReference ref)
    {
        ASSERT_WT();

        if (ref.size == 0) return {};

        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return {};

        std::lock_guard lock(m_lock);

        auto it = m_freeSlots.find(ref.size);
        if (it == m_freeSlots.end()) {
            m_freeSlots[ref.size] = std::vector<uint32_t>{ ref.offset };
        }
        else {
            it->second.push_back(ref.offset);
        }

        return {};
    }

    std::span<const glm::mat4> RigNodeRegistry::getRange(
        const util::BufferReference ref) const noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return std::span<glm::mat4>{};

        return std::span{ m_transforms }.subspan(ref.offset, ref.size);
    }

    std::span<glm::mat4> RigNodeRegistry::modifyRange(
        util::BufferReference ref) noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return std::span<glm::mat4>{};

        return std::span{ m_transforms }.subspan(ref.offset, ref.size);
    }

    void RigNodeRegistry::markDirtyAll() noexcept
    {
        m_dirtySlots.clear();
        markDirty({ 0, static_cast<uint32_t>(m_transforms.size()) });
    }

    void RigNodeRegistry::markDirty(
        util::BufferReference ref) noexcept
    {
        if (ref.size == 0) return;

        std::lock_guard lock(m_lockDirty);

        const auto& it = std::find_if(
            m_dirtySlots.begin(),
            m_dirtySlots.end(),
            [&ref](const auto& old) {
                return old == ref;
            });
        if (it != m_dirtySlots.end()) return;

        m_dirtySlots.push_back(ref);
    }

    uint32_t RigNodeRegistry::getActiveCount() const noexcept
    {
        return static_cast<uint32_t>(m_transforms.size());
    }

    void RigNodeRegistry::updateWT()
    {
    }
}
