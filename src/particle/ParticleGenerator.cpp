#include "ParticleGenerator.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "Particle.h"
#include "ParticleSystem.h"

#include "registry/MaterialRegistry.h"

namespace {
    float prnd(float max) {
        // https://stackoverflow.com/questions/686353/random-float-number-generation
        float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / max));
        return r2;
    }
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

        const auto& transform = node.getTransform();
        glm::vec3 pos = transform.getWorldPosition();

        for (int i = 0; i < 100; i++) {
            Particle particle;

            particle.m_pos = {
                pos.x + 50.f - prnd(100.f),
                pos.y + 10.f - prnd(20.f),
                pos.z + 50.f - prnd(100.f) };

            particle.m_dir = { 0.f, 1.f, 0.f };
            particle.m_velocity = 0.01f + prnd(.4f);
            particle.m_lifetime = 5.f + prnd(60.f);
            particle.m_scale = 0.0001f + prnd(0.2f);
            particle.m_materialIndex = m_material.m_registeredIndex;

            const int idx = floor(prnd(m_material.spriteCount));

            particle.m_spriteIndex = idx;
            particle.m_spriteCount = m_material.spriteCount;

            ps.addParticle(particle);
        }
    }
}
