#include "ParticleGenerator.h"

#include "engine/UpdateContext.h"

#include "Particle.h"
#include "ParticleSystem.h"

#include "registry/MaterialRegistry.h"

namespace {
    float prnd(float max) {
        // https://stackoverflow.com/questions/686353/random-float-number-generation
        float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / max));
        return r2;
    }

    int materialIndex = 0;
}

namespace particle {

    ParticleGenerator::ParticleGenerator()
    {}

    ParticleGenerator::~ParticleGenerator() = default;

    void ParticleGenerator::prepareWT()
    {
        MaterialRegistry::get().registerMaterial(m_material);
    }

    void ParticleGenerator::updateWT(const UpdateContext& ctx)
    {
        float x = m_definition.particlesPerSec * ctx.m_clock.elapsedSecs;

        Particle particle;
        particle.m_pos = { 50.f - prnd(100.f), 10.f - prnd(20.f), 50.f - prnd(100.f) };
        particle.m_dir = { 0.f, 1.f, 0.f};
        particle.m_velocity = 0.01f + prnd(0.8f);
        particle.m_lifetime = 10.f + prnd(500.f);
        particle.m_scale = 0.001f + prnd(0.5f);
        particle.m_materialIndex = (m_material.m_registeredIndex * rand()) % 10;
        particle.m_materialIndex = materialIndex;

        ParticleSystem::get().addParticle(particle);

        materialIndex++;
        if (materialIndex > 30)
            materialIndex = 0;
    }


}
