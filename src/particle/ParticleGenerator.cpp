#include "ParticleGenerator.h"

#include "util/Util.h"

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
    }

    void ParticleGenerator::updateWT(
        const UpdateContext& ctx,
        Node& node)
    {
        auto& ps = ParticleSystem::get();
        //if (ps.isFull()) return;

        const int freespace = ps.getFreespace();
        int count = m_definition.rate * ctx.m_clock.elapsedSecs;
        count = std::min(count, freespace);

        const auto& state = node.getState();
        glm::vec3 pos = state.getWorldPosition();

        for (int i = 0; i < count; i++) {
            Particle particle;

            particle.m_pos = {
                pos.x + 50.f - util::prnd(100.f),
                pos.y + 10.f - util::prnd(20.f),
                pos.z + 50.f - util::prnd(100.f) };

            particle.m_dir = { util::prnd(.4f), 1.f, util::prnd(.4f) };
            particle.m_velocity = 0.01f + util::prnd(.4f);
            particle.m_lifetime = 5.f + util::prnd(100.f);
            particle.m_scale = 0.0001f + util::prnd(0.2f);
            particle.m_materialIndex = m_material.m_registeredIndex;

            // NOTE KI start from idx, use full 0..spriteCount range
            const float idx = util::prnd(m_material.spriteCount);
            particle.m_spriteSpeed = 20.f - util::prnd(40.f);
            particle.m_spriteActiveIndex = idx;
            particle.m_spriteBaseIndex = 0;
            particle.m_spriteCount = m_material.spriteCount;

            if (!ps.addParticle(particle)) break;
        }
    }
}
