#include "BoneRegistry.h"

#include "asset/Assets.h"

#include "util/thread.h"

namespace {
}

namespace animation
{
    BoneRegistry::BoneRegistry()
    {
    }

    BoneRegistry::~BoneRegistry() = default;

    void BoneRegistry::clear()
    {
        ASSERT_WT();

        m_updateReady = false;

        m_transforms.clear();
        m_dirtyTransform.clear();

        m_snapshot.clear();
        m_dirtySnapshot.clear();

        // NOTE KI null entry
        reserveInstance(1);
    }

    void BoneRegistry::shutdown()
    {
        ASSERT_WT();

        clear();
    }

    void BoneRegistry::prepare()
    {
        ASSERT_WT();

        clear();
    }

    uint32_t BoneRegistry::reserveInstance(size_t count)
    {
        if (count == 0) return 0;

        size_t index;
        {
            std::lock_guard lock(m_lock);

            index = m_transforms.size();
            m_transforms.resize(m_transforms.size() + count);
            for (int i = 0; i < count; i++) {
                m_transforms[index + i] = glm::mat4{ 1.f };
            }

            markDirty(index, count);
        }

        return static_cast<uint32_t>(index);
    }

    std::span<glm::mat4> BoneRegistry::modifyRange(
        uint32_t start,
        size_t count) noexcept
    {
        return std::span{ m_transforms }.subspan(start, count);
    }

    void BoneRegistry::markDirtyAll() noexcept
    {
        markDirty(0, m_transforms.size());
    }

    void BoneRegistry::markDirty(size_t start, size_t count) noexcept
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

    uint32_t BoneRegistry::getActiveCount() const noexcept
    {
        return static_cast<uint32_t>(m_snapshot.size());
    }

    void BoneRegistry::updateWT()
    {
        makeSnapshot();
    }

    void BoneRegistry::makeSnapshot()
    {
        std::lock_guard lock(m_lock);
        std::lock_guard lockDirty(m_lockDirty);

        if (m_dirtyTransform.empty()) return;

        for (const auto& range : m_dirtyTransform) {
            const auto baseIndex = range.first;
            const auto updateCount = range.second;

            const size_t totalCount = m_transforms.size();
            if (m_snapshot.size() != totalCount) {
                m_snapshot.resize(totalCount);
            }

            for (size_t i = 0; i < updateCount; i++) {
                m_snapshot[baseIndex + i] = m_transforms[baseIndex + i];
            }

            m_dirtySnapshot.emplace_back(static_cast<uint32_t>(baseIndex), static_cast<uint32_t>(updateCount));
        }

        m_dirtyTransform.clear();
        m_updateReady = true;
    }
}
