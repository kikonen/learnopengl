#include "ParticleSystem.h"

#include "asset/Shader.h"
#include "asset/Program.h"

#include "component/Camera.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "render/Batch.h"

#include "registry/Registry.h"
#include "registry/ProgramRegistry.h"

#include "model/Particle.h"


ParticleSystem::ParticleSystem()
{
}

void ParticleSystem::prepareRT(
    const Assets& assets,
    Registry* registry)
{
    if (m_prepared) return;
    m_prepared = true;

    particleProgram = registry->m_programRegistry->getProgram(SHADER_PARTICLE, {});
    particleProgram->prepareRT(assets);
}

void ParticleSystem::update(const UpdateContext& ctx)
{
}

void ParticleSystem::bind(const RenderContext& ctx)
{
}

void ParticleSystem::render(const RenderContext& ctx)
{
    ctx.m_state.setEnabled(GL_BLEND, true);
    ctx.m_state.setEnabled(GL_CULL_FACE, false);

    for (auto& w : particles) {
        //Program* program;// = t->bind(ctx, nullptr);
        //if (!program) continue;
        //program->shadowMap.set(assets.shadowMapUnitIndex);

        //Batch& batch = t->batch;
        //batch.bind(ctx, program);

        //batch.draw(ctx, e, program);
    }

    ctx.bindDefaults();
}

void ParticleSystem::addParticle(const Particle& particle)
{
//    particles.push_back(particle);
}
