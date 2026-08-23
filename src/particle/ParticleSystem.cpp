#include "ParticleSystem.h"

#include <algorithm>
#include <execution>

#include "asset/Assets.h"

#include "util/thread.h"

#include "shader/SSBO.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "render/RenderContext.h"
#include "debug/DebugContext.h"

#include "registry/Registry.h"

#include "ParticlePool.h"
#include "Particle.h"
#include "ParticleSSBO.h"
#include "ParticleGenerator.h"

#include "kigl/GLBuffer.h"

namespace {
    constexpr size_t BLOCK_SIZE = 10000;
    constexpr size_t MAX_BLOCK_COUNT = 1100;

    constexpr int SKIP_FRAMES = 1;

    static particle::ParticleSystem* s_system{ nullptr };
}

namespace particle
{
    void ParticleSystem::init() noexcept
    {
        assert(!s_system);
        s_system = new ParticleSystem();
    }

    void ParticleSystem::release() noexcept
    {
        auto* s = s_system;
        s_system = nullptr;
        delete s;
    }

    ParticleSystem& ParticleSystem::get() noexcept
    {
        assert(s_system);
        return *s_system;
    }
}

namespace particle {
    ParticleSystem::ParticleSystem()
    {
        clear();
    }

    ParticleSystem::~ParticleSystem() = default;

    void ParticleSystem::clear()
    {
        m_pools.clear();
        m_pools.reserve(2);
        m_pools.emplace_back(std::make_unique<ParticlePool>("low"));
        m_pools.emplace_back(std::make_unique<ParticlePool>("high"));

        m_snapshotLocks.clear();
        m_snapshotLocks.reserve(2);
        m_snapshotLocks.emplace_back(std::make_unique<std::mutex>());
        m_snapshotLocks.emplace_back(std::make_unique<std::mutex>());
    }

    uint32_t ParticleSystem::getActiveParticleCount() const noexcept
    {
        uint32_t size = 0;
        for (const auto& pool : m_pools) {
            size += pool->getActiveParticleCount();
        }
        return size;
    }

    ParticlePool* ParticleSystem::getPool(uint32_t poolIndex)
    {
        return m_pools[poolIndex].get();
    }

    void ParticleSystem::prepare() {
        const auto& assets = Assets::get();

        m_enabled = assets.particleEnabled;

        for (auto& pool : m_pools) {
            pool->prepare();
        }

        // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
        m_ssbo.createEmpty(BLOCK_SIZE * sizeof(ParticleSSBO), kigl::getBufferStorageFlags());
        m_ssbo.map(kigl::getBufferMapFlags());

        m_ssbo.bindSSBO(SSBO_PARTICLES);
    }

    void ParticleSystem::beginFrame()
    {
        m_fence.waitFence();
    }

    void ParticleSystem::endFrame()
    {
        m_fence.setFence();
    }

    void ParticleSystem::updateWT(const UpdateContext& ctx)
    {
        const auto& dbg = ctx.getDebug();

        m_enabled = dbg.m_particleEnabled;

        if (!isEnabled()) return;

        {
            auto* pool = m_pools[m_updatePoolIndexWT].get();
            auto* mutex = m_snapshotLocks[m_updatePoolIndexWT].get();
            m_updatePoolIndexWT = (m_updatePoolIndexWT + 1) % m_pools.size();

            pool->updateWT(ctx);
            std::lock_guard lock(*mutex);
            pool->snapshotParticles();
        }
    }

    void ParticleSystem::updateRT(const UpdateContext& ctx)
    {
        m_enabled = ctx.getDebug().m_particleEnabled;

        if (!isEnabled()) return;

        auto poolIndex = m_updatePoolIndexRT;
        auto* pool = m_pools[m_updatePoolIndexRT].get();
        m_updatePoolIndexRT = (m_updatePoolIndexRT + 1) % m_pools.size();

        if (!pool->m_updateReady) return;

        m_frameSkipCount++;
        if (m_frameSkipCount < SKIP_FRAMES) {
            return;
        }
        m_frameSkipCount = 0;

        upload(poolIndex);
    }

    void ParticleSystem::upload(size_t poolIndex)
    {
        auto* pool = m_pools[poolIndex].get();
        auto* mutex = m_snapshotLocks[poolIndex].get();
        std::lock_guard lock(*mutex);

        bool resized = false;
        {
            size_t maxCount = 0;
            for (auto& pool : m_pools) {
                maxCount = std::max(maxCount, pool->m_snapshotCount);
            }
            resized = resizeBuffer(maxCount);

            m_pools[0]->m_baseIndex = 0;
            for (int i = 1; i < m_pools.size(); i++) {
                m_pools[i]->m_baseIndex = static_cast<uint32_t>(m_entryCount * i);
            }
        }

        {
            const auto totalCount = pool->m_snapshotCount;
            if (totalCount == 0) {
                pool->m_activeCount = 0;
                return;
            }

            const auto offset = pool->m_baseIndex * sizeof(ParticleSSBO);

            auto* __restrict mappedData = m_ssbo.mapped<ParticleSSBO>(offset);
            pool->upload(mappedData);

            // NOTE KI flush for explicit mode (no-op if using coherent mapping)
            m_ssbo.flushRange(offset, totalCount * sizeof(ParticleSSBO));
        }
    }

    bool ParticleSystem::resizeBuffer(size_t maxCount)
    {
        if (m_entryCount >= maxCount) return false;

        size_t blocks = (maxCount / BLOCK_SIZE) + 2;
        size_t entryCount = blocks * BLOCK_SIZE;

        // NOTE KI 2 pools
        // NOTE KI *reallocate* SSBO if needed
        m_ssbo.resizeBuffer(entryCount * 2 * sizeof(ParticleSSBO), true);

        m_ssbo.map(kigl::getBufferMapFlags());

        m_ssbo.bindSSBO(SSBO_PARTICLES);

        m_entryCount = entryCount;

        return true;
    }
}
