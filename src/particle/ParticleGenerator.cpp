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

        auto& ps = ParticleSystem::get();
        if (ps.isFull()) return;

        for (int i = 0; i < 1000; i++) {
            Particle particle;
            particle.m_pos = { 50.f - prnd(100.f), 10.f - prnd(20.f), 50.f - prnd(100.f) };
            particle.m_dir = { 0.f, 1.f, 0.f };
            particle.m_velocity = 0.01f + prnd(.4f);
            particle.m_lifetime = 10.f + prnd(200.f);
            particle.m_scale = 0.0001f + prnd(0.2f);
            //particle.m_materialIndex = (m_material.m_registeredIndex * rand()) % 10;
            //particle.m_materialIndex = materialIndex;
            particle.m_materialIndex = m_material.m_registeredIndex;

            const int rows = 8;
            const int cols = 8;
            const int max = rows * cols - 3;
            const int idx = floor(prnd(max));

            particle.m_index = idx;
            particle.m_cols = cols;
            particle.m_maxIndex = rows * cols - 3;


            ps.addParticle(particle);

            materialIndex++;
            if (materialIndex > 30)
                materialIndex = 0;
        }
    }
}
