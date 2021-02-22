#include "ParticleGenerator.h"

#include "asset/Assets.h"
#include "scene/ParticleSystem.h"

ParticleGenerator::ParticleGenerator(
	const Assets& assets,
	ParticleDefinition definition)
	: assets(assets),
	definition(definition)
{
}

void ParticleGenerator::update(const RenderContext& ctx)
{
	Particle particle;
	particle.pos = { 10, 10, 10 };
	particle.dir = { 0, 1, 0 };
	particle.velocity = 2;
	particle.lifetime = 5;

	system->addParticle(particle);
}
