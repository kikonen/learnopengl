#include "ParticleSystem.h"

#include "asset/Assets.h"

#include "shader/SSBO.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"

#include "Particle.h"
#include "ParticleGenerator.h"

namespace {
    constexpr size_t BLOCK_SIZE = 10000;
    constexpr size_t MAX_BLOCK_COUNT = 1100;

    static particle::ParticleSystem s_system;
}

namespace particle {
    ParticleSystem& ParticleSystem::get() noexcept
    {
        return s_system;
    }

    ParticleSystem::ParticleSystem()
    {
        m_particles.reserve(1 * BLOCK_SIZE);
    }

    void ParticleSystem::addParticle(const Particle& particle)
    {
        std::lock_guard lock(m_pendingLock);
        // TODO KI how to check full here?
        //if (isFull()) return;

        m_pending.push_back(particle);
    }

    void ParticleSystem::prepare() {
        const auto& assets = Assets::get();

        m_enabled = assets.particleEnabled;
        m_maxCount = std::min<int>(assets.particleMaxCount, MAX_BLOCK_COUNT * BLOCK_SIZE);

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useDebugFence = assets.glUseDebugFence;

        m_useMapped = false;
        m_useInvalidate = true;
        m_useFence = false;
        m_useDebugFence = false;

        if (!isEnabled()) return;

        m_ssbo.createEmpty(1 * BLOCK_SIZE * sizeof(ParticleSSBO), GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.bindSSBO(SSBO_PARTICLES);
    }

    void ParticleSystem::updateWT(const UpdateContext& ctx)
    {
        if (!isEnabled()) return;

        std::lock_guard lock(m_lock);
        preparePending();

        size_t size = m_particles.size();
        for (size_t i = 0; i < size; i++) {
            auto& particle = m_particles[i];
            if (!particle.update(ctx)) {
                if (i < size - 1) {
                    m_particles[i] = m_particles[size - 1];
                }
                size--;
                i--;
            }
        }

        if (size != m_particles.size()) {
            m_particles.resize(size);
        }

        snapshotParticles();
    }

    void ParticleSystem::updateRT(const UpdateContext& ctx)
    {
        if (!isEnabled()) return;
        if (!m_updateReady) return;

        m_frameSkipCount++;
        if (m_frameSkipCount < 2) {
            return;
        }
        m_frameSkipCount = 0;

        updateParticleBuffer();
    }

    void ParticleSystem::preparePending()
    {
        std::lock_guard lock(m_pendingLock);

        for (const auto& particle : m_pending) {
            if (isFull()) break;
            m_particles.push_back(particle);
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

    void ParticleSystem::updateParticleBuffer()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_snapshotCount == 0) {
            m_activeCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(ParticleSSBO);
        const size_t totalCount = m_snapshotCount;

        if (m_ssbo.m_size < totalCount * sz) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t bufferSize = blocks * BLOCK_SIZE * sz;
            if (m_ssbo.resizeBuffer(bufferSize)) {
                m_ssbo.bindSSBO(SSBO_PARTICLES);
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
