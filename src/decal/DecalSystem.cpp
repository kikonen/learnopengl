#include "DecalSystem.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "kigl/GLState.h"

#include "shader/SSBO.h"
#include "shader/ProgramRegistry.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "render/RenderContext.h"
#include "render/DebugContext.h"

#include "registry/Registry.h"

#include "Decal.h"
#include "DecalBuffer.h"

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
        : m_decalBuffer{ std::make_unique<DecalBuffer>(this) }
    {
    }

    DecalSystem::~DecalSystem() = default;

    void DecalSystem::clearWT()
    {
        ASSERT_WT();

        m_updateReady = false;

        m_decals.clear();
        m_snapshot.clear();

        m_snapshotCount = 0;
        m_activeCount = 0;

        m_decals.reserve(1 * BLOCK_SIZE);
    }

    void DecalSystem::shutdownWT()
    {
        ASSERT_WT();

        clearWT();
    }

    void DecalSystem::prepareWT() {
        ASSERT_WT();

        const auto& assets = Assets::get();

        m_enabled = assets.decalEnabled;
        m_maxCount = std::min<int>(assets.decalMaxCount, MAX_BLOCK_COUNT * BLOCK_SIZE);

        if (!isEnabled()) return;

        clearWT();
    }

    void DecalSystem::clearRT()
    {
        ASSERT_RT();

        m_decalBuffer->clear();
    }

    void DecalSystem::shutdownRT()
    {
        ASSERT_RT();

        m_decalBuffer->shutdown();
    }

    void DecalSystem::prepareRT()
    {
        ASSERT_RT();

        m_decalBuffer->prepare();
    }

    void DecalSystem::addDecal(const Decal& decal)
    {
        if (!decal.m_parent) return;

        std::lock_guard lock(m_lock);
        if (isFull()) return;

        m_decals.push_back(decal);
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

        if (ctx.m_dbg.m_decalEnabled) {
            snapshotDecals();
        }
    }

    void DecalSystem::updateRT(const UpdateContext& ctx)
    {
        m_decalBuffer->update(ctx);
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
}
