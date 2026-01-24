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

#include "Particle.h"
#include "ParticleSSBO.h"
#include "ParticleGenerator.h"

#include "kigl/GLBuffer.h"

namespace {
    constexpr size_t BLOCK_SIZE = 10000;
    constexpr size_t MAX_BLOCK_COUNT = 1100;

    // Threshold for parallel execution - below this, sequential is faster
    constexpr size_t PARALLEL_THRESHOLD = 5000;

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
        m_updateReady = false;

        m_particles.clear();
        m_particles.reserve(1 * BLOCK_SIZE);

        m_pending.clear();
        m_snapshot.clear();

        m_snapshotCount = 0;
        m_activeCount = 0;
    }

    bool ParticleSystem::isFull() const noexcept {
        return !m_enabled || m_particles.size() >= m_maxCount;
    }

    uint32_t ParticleSystem::getFreespace() const noexcept
    {
        std::lock_guard lock(m_pendingLock);

        uint32_t sz = static_cast<uint32_t>(m_maxCount - m_snapshotCount + m_pending.size());
        return std::max((uint32_t)0, sz);
    }

    bool ParticleSystem::addParticle(const Particle& particle)
    {
        // NOTE KI directly ignore invalid particles
        if (!particle.valid()) return true;

        std::lock_guard lock(m_pendingLock);

        if (!m_enabled && m_snapshotCount + m_pending.size() >= m_maxCount) return false;

        m_pending.push_back(particle);
        return true;
    }

    void ParticleSystem::prepare() {
        const auto& assets = Assets::get();

        m_enabled = assets.particleEnabled;
        m_maxCount = std::min<int>(assets.particleMaxCount, BLOCK_SIZE * MAX_BLOCK_COUNT);

        if (!isEnabled()) return;

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

        m_maxCount = std::min<int>(dbg.m_particleMaxCount, BLOCK_SIZE * MAX_BLOCK_COUNT);

        preparePending();

        const size_t size = m_particles.size();

        if (size >= PARALLEL_THRESHOLD) {
            // Parallel update
            std::for_each(
                std::execution::par,
                m_particles.begin(),
                m_particles.end(),
                [&ctx](Particle& p) { p.update(ctx); });

            // Parallel partition - move alive particles to front
            auto newEnd = std::partition(
                std::execution::par,
                m_particles.begin(),
                m_particles.end(),
                [](const Particle& p) { return p.isAlive(); });

            m_particles.erase(newEnd, m_particles.end());
        }
        else {
            // Sequential update for small counts
            std::for_each(
                m_particles.begin(),
                m_particles.end(),
                [&ctx](Particle& p) { p.update(ctx); });

            auto newEnd = std::partition(
                m_particles.begin(),
                m_particles.end(),
                [](const Particle& p) { return p.isAlive(); });

            m_particles.erase(newEnd, m_particles.end());
        }

        snapshotParticles();
    }

    void ParticleSystem::updateRT(const UpdateContext& ctx)
    {
        m_enabled = ctx.getDebug().m_particleEnabled;

        if (!isEnabled()) return;
        if (!m_updateReady) return;

        m_frameSkipCount++;
        if (m_frameSkipCount < 2) {
            return;
        }
        m_frameSkipCount = 0;

        upload();
    }

    void ParticleSystem::preparePending()
    {
        std::lock_guard lock(m_pendingLock);

        auto count = std::min(
            m_pending.size(),
            m_maxCount - m_particles.size());

        if (count > 0 && isEnabled()) {
            m_particles.insert(m_particles.end(), m_pending.begin(), m_pending.begin() + count);
        }
        m_pending.clear();
    }

    void ParticleSystem::snapshotParticles()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_particles.empty()) {
            m_snapshotCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(ParticleSSBO);
        const size_t totalCount = m_particles.size();

        if (m_snapshotCount != totalCount) {
            m_snapshot.resize(totalCount);
        }

        for (size_t i = 0; i < totalCount; i++) {
            m_particles[i].updateSSBO(m_snapshot[i]);
        }

        m_snapshotCount = totalCount;
        m_updateReady = true;
    }

    void ParticleSystem::upload()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_snapshotCount == 0) {
            m_activeCount = 0;
            return;
        }

        const size_t totalCount = m_snapshotCount;

        resizeBuffer(totalCount);

        auto* __restrict mappedData = m_ssbo.mapped<ParticleSSBO>(0);

        std::copy(
            std::begin(m_snapshot),
            std::end(m_snapshot),
            mappedData);

        // NOTE KI flush for explicit mode (no-op if using coherent mapping)
        m_ssbo.flushRange(0, totalCount * sizeof(ParticleSSBO));

        m_activeCount = totalCount;
        m_updateReady = false;
    }

    void ParticleSystem::resizeBuffer(size_t totalCount)
    {
        if (m_entryCount >= totalCount) return;

        size_t blocks = (totalCount / BLOCK_SIZE) + 2;
        size_t entryCount = blocks * BLOCK_SIZE;

        // NOTE KI *reallocate* SSBO if needed
        m_ssbo.resizeBuffer(entryCount * sizeof(ParticleSSBO), true);

        m_ssbo.map(kigl::getBufferMapFlags());

        m_ssbo.bindSSBO(SSBO_PARTICLES);

        m_entryCount = entryCount;
    }
}
