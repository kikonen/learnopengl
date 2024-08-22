#include "ParticleGenerator.h"

#include "util/Util.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "Particle.h"
#include "ParticleSystem.h"

#include "registry/MaterialRegistry.h"

namespace {
}

namespace particle {

    ParticleGenerator::ParticleGenerator()
    {}

    ParticleGenerator::~ParticleGenerator() = default;

    void ParticleGenerator::prepareWT()
    {
        MaterialRegistry::get().registerMaterial(m_material);
    }

    void ParticleGenerator::updateWT(
        const UpdateContext& ctx,
        Node& node)
    {
        float x = m_definition.particlesPerSec * ctx.m_clock.elapsedSecs;

        auto& ps = ParticleSystem::get();
        if (ps.isFull()) return;

        const auto& state = node.getState();
        glm::vec3 pos = state.getWorldPosition();

        for (int i = 0; i < 1000; i++) {
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

            ps.addParticle(particle);
        }
    }
}
