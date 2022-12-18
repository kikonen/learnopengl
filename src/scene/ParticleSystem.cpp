#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{
}

void ParticleSystem::prepare(const Assets& assets, ShaderRegistry& shaders)
{
    if (m_prepared) return;
    m_prepared = true;

    particleShader = shaders.getShader(assets, TEX_PARTICLE, {});
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
    }

    ctx.bindDefaults();
}

void ParticleSystem::addParticle(const Particle& particle)
{
//    particles.push_back(particle);
}
