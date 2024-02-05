#include "ParticleSystem.h"

#include "asset/Shader.h"
#include "asset/Program.h"

#include "kigl/GLState.h"

#include "component/Camera.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "render/Batch.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"
#include "registry/ProgramRegistry.h"

#include "model/Particle.h"


ParticleSystem::ParticleSystem()
{
}

void ParticleSystem::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

    particleProgram = ProgramRegistry::get().getProgram(SHADER_PARTICLE, {});
    particleProgram->prepareRT();
}

void ParticleSystem::update(const UpdateContext& ctx)
{
}

void ParticleSystem::bind(const RenderContext& ctx)
{
}

void ParticleSystem::render(const RenderContext& ctx)
{
    auto& state = ctx.m_state;

    state.setEnabled(GL_BLEND, true);
    state.setEnabled(GL_CULL_FACE, false);

    for (auto& w : particles) {
        //Program* program;// = t->bind(ctx, nullptr);
        //if (!program) continue;
        //program->shadowMap.set(assets.shadowMapUnitIndex);

        //auto& batch = t->batch;
        //batch.bind(ctx, program);

        //batch.draw(ctx, e, program);
    }

    ctx.bindDefaults();
}

void ParticleSystem::addParticle(const Particle& particle)
{
//    particles.push_back(particle);
}
