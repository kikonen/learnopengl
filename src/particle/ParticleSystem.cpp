#include "ParticleSystem.h"

#include "asset/Assets.h"
#include "asset/SSBO.h"

#include "kigl/GLState.h"

#include "component/Camera.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "render/Batch.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"
#include "registry/ProgramRegistry.h"

#include "Particle.h"
#include "ParticleGenerator.h"

namespace {
    constexpr size_t PARTICLE_BLOCK_SIZE = 1000;

    std::unique_ptr<particle::ParticleGenerator> generator;
}

namespace particle {
    ParticleSystem& ParticleSystem::get() noexcept
    {
        static ParticleSystem s_system;
        return s_system;
    }

    ParticleSystem::ParticleSystem()
    {
    }

    void ParticleSystem::addParticle(const Particle& particle)
    {
        std::lock_guard lock(m_lock);
        m_particles.push_back(particle);
    }

    void ParticleSystem::prepare() {
        const auto& assets = Assets::get();

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useDebugFence = assets.glUseDebugFence;

        m_useMapped = false;
        m_useInvalidate = true;
        m_useFence = false;
        m_useDebugFence = false;

        generator = std::make_unique<particle::ParticleGenerator>();
        generator->prepareWT();

        m_ssbo.createEmpty(1000 * PARTICLE_BLOCK_SIZE * sizeof(ParticleSSBO), GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.bindSSBO(SSBO_PARTICLES);
    }

    void ParticleSystem::updateWT(const UpdateContext& ctx)
    {
        if (m_particles.size() < 100000) {
            for (int i = 0; i < 1000; i++) {
                generator->updateWT(ctx);
            }
        }

        std::lock_guard lock(m_lock);

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
    }

    void ParticleSystem::updateRT(const UpdateContext& ctx)
    {
        updateParticleBuffer();
    }

    void ParticleSystem::updateParticleBuffer()
    {
        std::lock_guard lock(m_lock);

        if (m_particles.empty()) {
            m_activeCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(ParticleSSBO);
        const size_t totalCount = m_particles.size();

        m_entries.reserve(m_particles.size());
        while (m_entries.size() < m_particles.size()) {
            m_entries.emplace_back();
        }

        for (size_t i = 0; i < m_particles.size(); i++) {
            m_entries[i] = m_particles[i].toSSBO();
        }

        if (m_ssbo.m_size < totalCount * sz) {
            size_t blocks = (totalCount / PARTICLE_BLOCK_SIZE) + 2;
            size_t bufferSize = blocks * PARTICLE_BLOCK_SIZE * sz;
            m_ssbo.resizeBuffer(bufferSize);
        }

        m_ssbo.invalidateRange(
            0,
            totalCount * sz);

        m_ssbo.update(
            0,
            totalCount * sz,
            m_entries.data());

        m_activeCount = totalCount;
    }
}
