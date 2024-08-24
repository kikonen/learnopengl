#include "DecalSystem.h"

#include "asset/Assets.h"
#include "asset/SSBO.h"

#include "kigl/GLState.h"


#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/ProgramRegistry.h"

#include "Decal.h"

namespace {
    constexpr size_t BLOCK_SIZE = 10000;
    constexpr size_t MAX_BLOCK_COUNT = 1100;

    static decal::DecalSystem g_system;
}

namespace decal {
    DecalSystem& DecalSystem::get() noexcept
    {
        return g_system;
    }

    DecalSystem::DecalSystem()
    {
        m_decals.reserve(1 * BLOCK_SIZE);
    }

    void DecalSystem::addDecal(const Decal& decal)
    {
        if (!decal.m_parent) return;

        std::lock_guard lock(m_lock);
        if (isFull()) return;

        m_decals.push_back(decal);
    }

    void DecalSystem::prepare() {
        const auto& assets = Assets::get();

        m_enabled = assets.decalEnabled;
        m_maxCount = std::min<int>(assets.decalMaxCount, MAX_BLOCK_COUNT * BLOCK_SIZE);

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useDebugFence = assets.glUseDebugFence;

        m_useMapped = false;
        m_useInvalidate = true;
        m_useFence = false;
        m_useDebugFence = false;

        if (!isEnabled()) return;

        m_ssbo.createEmpty(1 * BLOCK_SIZE * sizeof(DecalSSBO), GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.bindSSBO(SSBO_DECALS);
    }

    void DecalSystem::updateWT(const UpdateContext& ctx)
    {
        if (!isEnabled()) return;

        std::lock_guard lock(m_lock);

        size_t size = m_decals.size();
        for (size_t i = 0; i < size; i++) {
            auto& decal = m_decals[i];
            if (!decal.update(ctx)) {
                if (i < size - 1) {
                    m_decals[i] = m_decals[size - 1];
                }
                size--;
                i--;
            }
        }

        if (size != m_decals.size()) {
            m_decals.resize(size);
        }

        snapshotDecals();
    }

    void DecalSystem::updateRT(const UpdateContext& ctx)
    {
        if (!isEnabled()) return;
        if (!m_updateReady) return;

        //m_frameSkipCount++;
        //if (m_frameSkipCount < 2) {
        //    return;
        //}
        //m_frameSkipCount = 0;

        updateDecalBuffer();
    }

    void DecalSystem::snapshotDecals()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_decals.empty()) {
            m_snapshotCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(DecalSSBO);
        const size_t totalCount = m_decals.size();

        if (m_snapshotCount != totalCount) {
            m_snapshot.resize(totalCount);
        }

        for (size_t i = 0; i < totalCount; i++) {
             m_decals[i].updateSSBO(m_snapshot[i]);
        }

        m_snapshotCount = totalCount;
        m_updateReady = true;
    }

    void DecalSystem::updateDecalBuffer()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_snapshotCount == 0) {
            m_activeCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(DecalSSBO);
        const size_t totalCount = m_snapshotCount;

        if (m_ssbo.m_size < totalCount * sz) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t bufferSize = blocks * BLOCK_SIZE * sz;
            if (m_ssbo.resizeBuffer(bufferSize)) {
                m_ssbo.bindSSBO(SSBO_DECALS);
            }
        }

        //m_ssbo.invalidateRange(
        //    0,
        //    totalCount * sz);

        //if (m_useInvalidate) {
        //    m_ssbo.invalidateRange(0, totalCount * sz);
        //}

        m_ssbo.update(
            0,
            totalCount * sz,
            m_snapshot.data());

        m_activeCount = totalCount;

        m_updateReady = false;
    }
}
