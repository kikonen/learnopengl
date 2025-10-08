#include "SocketRegistry.h"

#include "util/thread.h"

#include "asset/Assets.h"

#include "SocketTransformSSBO.h"

namespace {
    constexpr int INITIAL_SIZE = 10000;
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
        m_dirtyTransform.clear();

        m_snapshot.clear();
        m_dirtySnapshot.clear();

        m_transforms.reserve(INITIAL_SIZE);
        m_dirtyTransform.reserve(INITIAL_SIZE);
        m_snapshot.reserve(INITIAL_SIZE);
        m_dirtySnapshot.reserve(INITIAL_SIZE);

        // NOTE KI null entry
        addInstance(1);
    }

    void SocketRegistry::prepare()
    {
        ASSERT_RT();

        const auto& assets = Assets::get();

        clear();
    }

    uint32_t SocketRegistry::addInstance(size_t count)
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

    void SocketRegistry::removeInstance(
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

    std::span<glm::mat4> SocketRegistry::modifyRange(
        uint32_t start,
        size_t count) noexcept
    {
        // NOTE KI modifying null socket is not allowed
        if (!start) return std::span<glm::mat4>{};

        return std::span{ m_transforms }.subspan(start, count);
    }

    void SocketRegistry::markDirtyAll() noexcept
    {
        markDirty(0, m_transforms.size());
    }

    void SocketRegistry::markDirty(size_t start, size_t count) noexcept
    {
        //ASSERT_WT();

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
