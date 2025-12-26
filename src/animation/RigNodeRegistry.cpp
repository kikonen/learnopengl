#include "RigNodeRegistry.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "JointTransformSSBO.h"

namespace {
    constexpr int INITIAL_SIZE = 10000;
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
        m_dirtyTransforms.clear();

        m_transforms.reserve(INITIAL_SIZE);
        m_dirtyTransforms.reserve(INITIAL_SIZE);

        // NOTE KI null entry
        addInstance(1);
    }

    void RigNodeRegistry::prepare()
    {
        ASSERT_RT();

        clear();
    }

    uint32_t RigNodeRegistry::addInstance(size_t count)
    {
        //ASSERT_WT();

        if (count == 0) return 0;

        uint32_t offset;
        {
            std::lock_guard lock(m_lock);

            auto it = m_freeSlots.find(count);
            if (it != m_freeSlots.end() && !it->second.empty()) {
                offset = it->second[it->second.size() - 1];
                it->second.pop_back();
            }
            else {
                offset = static_cast<uint32_t>(m_transforms.size());
                m_transforms.resize(m_transforms.size() + count);
            }

            for (int i = 0; i < count; i++) {
                m_transforms[offset + i] = glm::mat4{ 1.f };
            }

            markDirty({ offset, static_cast<uint32_t>(count) });
        }

        return static_cast<uint32_t>(offset);
    }

    void RigNodeRegistry::removeInstance(
        util::BufferReference ref)
    {
        ASSERT_WT();

        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return;

        std::lock_guard lock(m_lock);

        auto it = m_freeSlots.find(ref.size);
        if (it == m_freeSlots.end()) {
            m_freeSlots[ref.size] = std::vector<uint32_t>{ ref.offset };
        }
        else {
            it->second.push_back(ref.offset);
        }
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
        markDirty({ 0, static_cast<uint32_t>(m_transforms.size()) });
    }

    void RigNodeRegistry::markDirty(
        util::BufferReference ref) noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (ref.offset == 0) return;

        std::lock_guard lock(m_lockDirty);

        const auto& it = std::find_if(
            m_dirtyTransforms.begin(),
            m_dirtyTransforms.end(),
            [&ref](const auto& old) {
                return old == ref;
            });
        if (it != m_dirtyTransforms.end()) return;

        m_dirtyTransforms.push_back(ref);
    }

    uint32_t RigNodeRegistry::getActiveCount() const noexcept
    {
        return static_cast<uint32_t>(m_transforms.size());
    }

    void RigNodeRegistry::updateWT()
    {
    }
}
