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
        glm::vec3 pos = state.getWorldPosition();

        const auto& rnd = *m_random.get();

        for (int i = 0; i < count; i++) {
            Particle particle;

            {
                const auto var = df.m_areaVariation;
                const float variationX = var * rnd.rnd();
                const float variationY = var * rnd.rnd();
                const float variationZ = var * rnd.rnd();

                particle.m_pos = {
                    pos.x - var + 2.f * variationX,
                    pos.y - var + 2.f * variationY,
                    pos.z - var + 2.f * variationZ
                };
            }

            {
                const auto& normal = df.m_dir;
                const auto var = df.m_dirVariation;
                const auto variation = glm::radians(var * rnd.rnd());
                const float theta = glm::radians(360 * rnd.rnd());

                particle.m_dir = df.m_dir;
            }
            {
                const auto velocity = df.m_velocity;
                const auto var = df.m_velocityVariation;
                const auto variation = var * rnd.rnd();
                particle.m_velocity = velocity - var + 2.f * variation;
            }
            {
                const auto lifetime = df.m_lifetime;
                const auto var = df.m_lifetimeVariation;
                const auto variation = var * rnd.rnd();
                particle.m_lifetime = lifetime - var + 2.f * variation;
            }
            {
                const auto size = df.m_size;
                const auto var = df.m_sizeVariation;
                const auto variation = var * rnd.rnd();
                particle.m_scale = size - var + 2.f * variation;
            }
            {
                // const auto sprite = df.m_sprite;
                // const auto var = df.m_sizeVariation;
                // const auto variation = var * rnd.rnd();
                // particle.m_s = size - var + 2.f * variation;
            }

            particle.m_materialIndex = m_material.m_registeredIndex;

            {
                particle.m_spriteBaseIndex = df.m_spriteBase;
                particle.m_spriteCount = df.m_spriteCount;
            }
            {
                // NOTE KI start from idx, use full 0..spriteCount range
                const float idx = rnd.rnd(m_material.spriteCount);

                const auto base = df.m_spriteBase;
                const auto var = df.m_spriteBaseVariation;
                const auto variation = var * rnd.rnd();
                particle.m_spriteActiveIndex = base - var + 2.f * variation;
            }
            {
                const auto speed = df.m_spriteSpeed;
                const auto var = df.m_spriteSpeedVariation;
                const auto variation = var * rnd.rnd();
                particle.m_spriteSpeed = speed - var + 2.f * variation;
            }

            if (!ps.addParticle(particle)) break;
        }
    }
}
