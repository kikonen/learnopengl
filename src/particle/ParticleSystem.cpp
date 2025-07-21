#include "ParticleSystem.h"

#include <algorithm>

#include "asset/Assets.h"

#include "kigl/GLSyncQueue_impl.h"

#include "shader/SSBO.h"
#include "shader/ProgramRegistry.h"

#include "kigl/GLState.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "render/RenderContext.h"
#include "render/DebugContext.h"

#include "registry/Registry.h"

#include "Particle.h"
#include "ParticleSSBO.h"
#include "ParticleGenerator.h"

namespace {
    constexpr size_t BLOCK_SIZE = 10000;
    constexpr size_t MAX_BLOCK_COUNT = 1100;

    constexpr size_t RANGE_COUNT = 3;

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
        m_lastParticleSize = 0;
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
        m_maxCount = std::min<int>(assets.particleMaxCount, MAX_BLOCK_COUNT * BLOCK_SIZE);

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useFenceDebug = assets.glUseFenceDebug;

        m_useMapped = true;
        m_useInvalidate = false;
        m_useFence = true;
        m_useFenceDebug = true;

        if (!isEnabled()) return;
    }

    void ParticleSystem::updateWT(const UpdateContext& ctx)
    {
        const auto& dbg = ctx.m_dbg;

        m_enabled = dbg.m_particleEnabled;

        if (!isEnabled()) return;

        m_maxCount = std::min<int>(dbg.m_particleMaxCount, MAX_BLOCK_COUNT * BLOCK_SIZE);

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
        m_enabled = ctx.m_dbg.m_particleEnabled;

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

        auto count = std::min(
            m_pending.size(),
            m_maxCount - m_particles.size());

        if (count > 0 && isEnabled()) {
            //KI_INFO_OUT(fmt::format("PS: pending={}, copy={}, size={}", m_pending.size(), count, m_particles.size()));
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

    void ParticleSystem::updateParticleBuffer()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_snapshotCount == 0) {
            m_activeCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(ParticleSSBO);
        const size_t totalCount = m_snapshotCount;

        createParticleBuffer();

        //m_ssbo.invalidateRange(
        //    0,
        //    totalCount * sz);

        //if (m_useInvalidate) {
        //    m_ssbo.invalidateRange(0, totalCount * sz);
        //}

        auto& current = m_queue->current();
        auto* mappedData = m_queue->m_buffer.mapped< ParticleSSBO>(current.m_baseOffset);

        m_queue->waitFence();
        std::copy(
            std::begin(m_snapshot),
            std::end(m_snapshot),
            mappedData);
        m_queue->setFence();

        m_activeCount = totalCount;

        m_queue->bindCurrentSSBO(SSBO_PARTICLES, false, totalCount);

        m_queue->next();

        m_updateReady = false;
    }

    void ParticleSystem::createParticleBuffer()
    {
        const size_t totalCount = m_snapshotCount;

        if (!m_queue || m_queue->getEntryCount() < totalCount) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t entryCount = blocks * BLOCK_SIZE;

            // NOTE KI OpenGL Insights - Chapter 28
            m_queue = std::make_unique<kigl::GLSyncQueue<ParticleSSBO>>(
                "particle_ssbo",
                entryCount,
                RANGE_COUNT,
                m_useMapped,
                m_useInvalidate,
                m_useFence,
                m_useFenceDebug);

            m_queue->prepare(1, false);
        }
    }
}
