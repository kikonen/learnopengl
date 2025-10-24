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
        m_dirtyTransform.clear();

        m_transforms.reserve(INITIAL_SIZE);
        m_dirtyTransform.reserve(INITIAL_SIZE);

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

        size_t index;
        {
            std::lock_guard lock(m_lock);

            auto it = m_freeSlots.find(count);
            if (it != m_freeSlots.end() && !it->second.empty()) {
                index = it->second[it->second.size() - 1];
                it->second.pop_back();
            }
            else {
                index = m_transforms.size();
                m_transforms.resize(m_transforms.size() + count);
            }

            for (int i = 0; i < count; i++) {
                m_transforms[index + i] = glm::mat4{ 1.f };
            }

            markDirty(index, count);
        }

        return static_cast<uint32_t>(index);
    }

    void RigNodeRegistry::removeInstance(
        uint32_t index,
        size_t count)
    {
        ASSERT_WT();

        // NOTE KI modifying null socket is not allowed
        assert(index > 0);

        std::lock_guard lock(m_lock);

        auto it = m_freeSlots.find(count);
        if (it == m_freeSlots.end()) {
            m_freeSlots[count] = std::vector<uint32_t>{ index };
        }
        else {
            it->second.push_back(index);
        }
    }

    std::span<glm::mat4> RigNodeRegistry::modifyRange(
        uint32_t start,
        size_t count) noexcept
    {
        // NOTE KI modifying null socket is not allowed
        assert(start > 0);

        return std::span{ m_transforms }.subspan(start, count);
    }

    void RigNodeRegistry::markDirtyAll() noexcept
    {
        markDirty(0, m_transforms.size());
    }

    void RigNodeRegistry::markDirty(size_t start, size_t count) noexcept
    {
        if (count == 0) return;

        std::lock_guard lock(m_lockDirty);

        const auto& it = std::find_if(
            m_dirtyTransform.begin(),
            m_dirtyTransform.end(),
            [&start, &count](const auto& pair) {
                return pair.first == start && pair.second == count;
            });
        if (it != m_dirtyTransform.end()) return;

        m_dirtyTransform.emplace_back(static_cast<uint32_t>(start), static_cast<uint32_t>(count));
    }

    uint32_t RigNodeRegistry::getActiveCount() const noexcept
    {
        return static_cast<uint32_t>(m_transforms.size());
    }

    void RigNodeRegistry::updateWT()
    {
    }
}
