#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(const Assets& assets)
	: assets(assets)
{
	particleShader = Shader::getShader(assets, TEX_TEXTURE);
	particleShader->prepare();
}

void ParticleSystem::prepare()
{
}

void ParticleSystem::update(RenderContext& ctx)
{
}

void ParticleSystem::bind(RenderContext& ctx)
{
}

void ParticleSystem::render(RenderContext& ctx)
{
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	for (auto& w : particles) {
		//Shader* shader;// = t->bind(ctx, nullptr);
		//if (!shader) continue;
		//shader->shadowMap.set(assets.shadowMapUnitIndex);

		//Batch& batch = t->batch;
		//batch.bind(ctx, shader);

		//batch.draw(ctx, e, shader);

		//batch.flush(ctx, t);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void ParticleSystem::addParticle(const Particle& particle)
{
//	particles.push_back(particle);
}
