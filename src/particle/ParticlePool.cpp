#include "ParticlePool.h"

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

namespace
{
    constexpr size_t BLOCK_SIZE = 10000;
    constexpr size_t MAX_BLOCK_COUNT = 1100;

    // Threshold for parallel execution - below this, sequential is faster
    constexpr size_t PARALLEL_THRESHOLD = 5000;
}

namespace particle
{
    ParticlePool::ParticlePool(const std::string& name)
    {
    }

    ParticlePool::~ParticlePool() = default;

    void ParticlePool::clear()
    {
        m_updateReady = false;

        m_particles.clear();
        m_particles.reserve(1 * BLOCK_SIZE);

        m_pending.clear();
        m_snapshot.clear();

        m_snapshotCount = 0;
        m_activeCount = 0;
    }

    void ParticlePool::prepare()
    {
        const auto& assets = Assets::get();

        m_maxCount = std::min<int>(assets.particleMaxCount, BLOCK_SIZE * MAX_BLOCK_COUNT);
    }

    bool ParticlePool::isFull() const noexcept
    {
        return m_particles.size() >= m_maxCount;
    }

    uint32_t ParticlePool::getFreespace() const noexcept
    {
        std::lock_guard lock(m_pendingLock);

        uint32_t sz = static_cast<uint32_t>(m_maxCount - m_snapshotCount + m_pending.size());
        return std::max((uint32_t)0, sz);
    }

    bool ParticlePool::addParticle(const Particle& particle)
    {
        // NOTE KI directly ignore invalid particles
        if (!particle.valid()) return true;

        std::lock_guard lock(m_pendingLock);

        if (m_snapshotCount + m_pending.size() >= m_maxCount) return false;

        m_pending.push_back(particle);
        return true;
    }

    void ParticlePool::updateWT(const UpdateContext& ctx)
    {
        const auto& dbg = ctx.getDebug();

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
        }
        else {
            // Sequential update for small counts
            std::for_each(
                m_particles.begin(),
                m_particles.end(),
                [&ctx](Particle& p) { p.update(ctx); });
        }

        // NOTE KI linear scan for compact; more cache line friendly
        {
            size_t write = 0;
            for (size_t read = 0; read < m_particles.size(); ++read) {
                if (m_particles[read].isAlive()) {
                    if (write != read) m_particles[write] = m_particles[read];
                    ++write;
                }
            }
            m_particles.resize(write);
        }
    }

    void ParticlePool::preparePending()
    {
        std::lock_guard lock(m_pendingLock);

        auto count = std::min(
            m_pending.size(),
            m_maxCount - m_particles.size());

        if (count > 0) {
            m_particles.insert(m_particles.end(), m_pending.begin(), m_pending.begin() + count);
        }
        m_pending.clear();
    }

    void ParticlePool::snapshotParticles()
    {
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

    void ParticlePool::upload(ParticleSSBO* mappedData)
    {
        if (m_snapshotCount == 0) {
            m_activeCount = 0;
            return;
        }

        const size_t totalCount = m_snapshotCount;

        std::copy(
            std::begin(m_snapshot),
            std::end(m_snapshot),
            mappedData);

        m_activeCount = totalCount;
        m_updateReady = false;
    }
}
