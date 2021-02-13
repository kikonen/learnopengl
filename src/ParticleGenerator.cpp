#include "ParticleGenerator.h"

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
