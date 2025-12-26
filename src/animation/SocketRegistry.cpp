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
        m_dirtyTransforms.clear();

        m_snapshot.clear();
        m_dirtySnapshot.clear();

        m_transforms.reserve(INITIAL_SIZE);
        m_dirtyTransforms.reserve(INITIAL_SIZE);
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

    void SocketRegistry::removeInstance(
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
        markDirty({ 0, static_cast<uint32_t>(m_transforms.size()) });
    }

    void SocketRegistry::markDirty(
        util::BufferReference ref) noexcept
    {
        //ASSERT_WT();

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

        if (m_dirtyTransforms.empty()) return;

        for (const auto& range : m_dirtyTransforms) {
            const auto baseIndex = range.offset;
            const auto updateCount = range.size;

            const size_t totalCount = m_transforms.size();
            if (m_snapshot.size() != totalCount) {
                m_snapshot.resize(totalCount);
            }

            for (size_t i = 0; i < updateCount; i++) {
                m_snapshot[baseIndex + i] = m_transforms[baseIndex + i];
            }

            m_dirtySnapshot.emplace_back(static_cast<uint32_t>(baseIndex), static_cast<uint32_t>(updateCount));
        }

        m_dirtyTransforms.clear();
        m_updateReady = true;
    }
}
