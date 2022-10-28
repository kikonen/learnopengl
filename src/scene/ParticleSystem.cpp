#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{
}

void ParticleSystem::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    particleShader = shaders.getShader(assets, TEX_PARTICLE, PARTICLE_MATERIAL_COUNT, {});
    particleShader->prepare(assets);
}

void ParticleSystem::update(const RenderContext& ctx)
{
}

void ParticleSystem::bind(const RenderContext& ctx)
{
}

void ParticleSystem::render(const RenderContext& ctx)
{
    ctx.state.enable(GL_BLEND);
    ctx.state.disable(GL_CULL_FACE);

    for (auto& w : particles) {
        //Shader* shader;// = t->bind(ctx, nullptr);
        //if (!shader) continue;
        //shader->shadowMap.set(assets.shadowMapUnitIndex);

        //Batch& batch = t->batch;
        //batch.bind(ctx, shader);

        //batch.draw(ctx, e, shader);

        //batch.flush(ctx, t);
    }

    ctx.state.enable(GL_CULL_FACE);
    ctx.state.disable(GL_BLEND);
}

void ParticleSystem::addParticle(const Particle& particle)
{
//    particles.push_back(particle);
}
