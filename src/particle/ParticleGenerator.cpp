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
        m_material.registerMaterial();
        m_random = std::make_unique<util::Random>(m_definition.m_seed);
    }

    void ParticleGenerator::updateWT(
        const UpdateContext& ctx,
        Node& node)
    {
        auto& ps = ParticleSystem::get();
        //if (ps.isFull()) return;

        const auto& df = m_definition;

        m_pendingCount += df.m_rate * ctx.m_clock.elapsedSecs;

        if (m_pendingCount < 1.f) return;

        const int freespace = ps.getFreespace();
        const int count = std::min(
            static_cast<int>(m_pendingCount),
            freespace);

        m_pendingCount -= count;
        if (count == 0) return;

        const auto& state = node.getState();
        //glm::vec3 pos = state.getWorldPosition();

        const auto& rnd = *m_random.get();

        for (int i = 0; i < count; i++) {
            Particle particle;

            {
                particle.m_materialIndex = m_material.m_registeredIndex;

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

            if (!ps.addParticle(particle)) break;
        }
    }
}
