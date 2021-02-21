#include "ParticleGenerator.h"

#include "asset/Assets.h"
#include "scene/ParticleSystem.h"

ParticleGenerator::ParticleGenerator(
	const Assets& assets, 
	ParticleSystem* system, 
	ParticleDefinition definition)
	: assets(assets),
	system(system),
	definition(definition)
{
}

void ParticleGenerator::generate(const RenderContext& ctx)
{
}
