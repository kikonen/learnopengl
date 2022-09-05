#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(const Assets& assets)
	: assets(assets)
{
}

void ParticleSystem::prepare(ShaderRegistry& shaders)
{
	particleShader = shaders.getShader(assets, TEX_TEXTURE);
	particleShader->prepare();
}

void ParticleSystem::update(RenderContext& ctx)
{
}

void ParticleSystem::bind(RenderContext& ctx)
{
}

void ParticleSystem::render(RenderContext& ctx)
{
	ctx.state.enable(GL_BLEND);
	ctx.state.disable(GL_CULL_FACE);

	for (auto& w : particles) {
		//std::shared_ptr<Shader> shader;// = t->bind(ctx, nullptr);
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
//	particles.push_back(particle);
}
