#include "SocketRegistry.h"

#include "asset/Assets.h"
#include "asset/SSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 1000;
    constexpr size_t MAX_BLOCK_COUNT = 5100;

    static animation::SocketRegistry s_registry;
}

namespace animation
{
    animation::SocketRegistry& SocketRegistry::get() noexcept
    {
        return s_registry;
    }

    SocketRegistry::SocketRegistry()
    {
        // NOTE KI null entry
        reserveInstance(1);
    }

    SocketRegistry::~SocketRegistry() = default;

    void SocketRegistry::prepare()
    {
        const auto& assets = Assets::get();

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useDebugFence = assets.glUseDebugFence;

        m_frameSkipCount = 1;

        m_ssbo.createEmpty(1 * BLOCK_SIZE * sizeof(glm::mat4), GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.bindSSBO(SSBO_SOCKET_TRANSFORMS);
    }

    uint32_t SocketRegistry::reserveInstance(size_t count)
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

    std::span<glm::mat4> SocketRegistry::modifyRange(
        uint32_t start,
        size_t count) noexcept
    {
        return std::span{ m_transforms }.subspan(start, count);
    }

    void SocketRegistry::markDirtyAll() noexcept
    {
        markDirty(0, m_transforms.size());
    }

    void SocketRegistry::markDirty(size_t start, size_t count) noexcept
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

    uint32_t SocketRegistry::getActiveCount() const noexcept
    {
        return static_cast<uint32_t>(m_snapshot.size());
    }

    void SocketRegistry::updateWT()
    {
        makeSnapshot();
    }

    void SocketRegistry::updateRT()
    {
        if (!m_updateReady) return;

        m_frameSkipCount++;
        if (m_frameSkipCount < 2) {
            return;
        }
        m_frameSkipCount = 0;

        updateBuffer();
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

    void SocketRegistry::updateBuffer()
    {
        std::lock_guard lock(m_lock);

        if (m_dirtySnapshot.empty()) return;

        for (const auto& range : m_dirtySnapshot) {
            if (updateSpan(range.first, range.second)) break;
        }

        m_dirtySnapshot.clear();
        m_updateReady = false;
    }

    bool SocketRegistry::updateSpan(
        size_t updateIndex,
        size_t updateCount)
    {
        constexpr size_t sz = sizeof(glm::mat4);
        const size_t totalCount = m_snapshot.size();

        if (totalCount == 0) return true;

        if (m_ssbo.m_size < totalCount * sz) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t bufferSize = blocks * BLOCK_SIZE * sz;
            if (m_ssbo.resizeBuffer(bufferSize)) {
                m_ssbo.bindSSBO(SSBO_SOCKET_TRANSFORMS);
            }

            updateIndex = 0;
            updateCount = totalCount;
        }

        //if (m_useInvalidate) {
        //    m_ssbo.invalidateRange(updateIndex * sz, updateCount * sz);
        //}

        m_ssbo.update(
            updateIndex * sz,
            updateCount * sz,
            &m_snapshot[updateIndex]);

        return updateCount == totalCount;
    }
}
