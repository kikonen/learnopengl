#include "ParticleGenerator.h"

#include "util/util.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "Particle.h"
#include "ParticleSystem.h"


namespace {
}

namespace particle {

    ParticleGenerator::ParticleGenerator()
    {}

    ParticleGenerator::~ParticleGenerator() = default;

    void ParticleGenerator::prepareWT()
    {
        m_random = std::make_unique<util::Random>(m_definition.m_seed);
        m_materialIndex = m_definition.m_material->m_registeredIndex;
    }

    void ParticleGenerator::updateWT(
        const UpdateContext& ctx,
        model::Node& node)
    {
        if (m_requestedCount <= 0.f && m_pendingCount <= 0.f) return;

        auto& particleSystem = ParticleSystem::get();

        const auto& df = m_definition;

        float emitCount = std::min(m_requestedCount, df.m_rate * ctx.m_clock.elapsedSecs);
        m_requestedCount = std::max(m_requestedCount - emitCount, 0.f);

        m_pendingCount += emitCount;

        if (m_pendingCount < 1.f) return;

        const int freespace = particleSystem.getFreespace();
        const int count = std::min(
            static_cast<int>(m_pendingCount),
            freespace);

        m_pendingCount -= count;
        if (count == 0) return;

        if (!particleSystem.isEnabled()) return;

        const auto& state = node.getState();
        //glm::vec3 pos = state.getWorldPosition();

        const auto& rnd = *m_random.get();

        for (int i = 0; i < count; i++) {
            Particle particle;

            {
                particle.m_materialIndex = m_materialIndex;

                particle.m_gravity = df.m_gravity;

                const auto pos = glm::vec4{ df.randomPosition(rnd), 1.f };
                particle.m_pos = state.getModelMatrix() * pos;
                particle.m_velocity = df.randomDirection(rnd) * df.randomSpeed(rnd);
                particle.m_lifetime = df.randomLifetime(rnd);
                particle.m_scale = df.randomSize(rnd);

                particle.m_spriteBaseIndex = df.m_spriteBase;
                particle.m_spriteCount = df.m_spriteCount;
                particle.m_spriteActiveIndex = df.randomSpriteIndex(rnd);
                particle.m_spriteSpeed = df.randomSpriteSpeed(rnd);
            }

            if (!particleSystem.addParticle(particle)) break;
        }
    }
}
