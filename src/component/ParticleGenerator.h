#pragma once

#include "asset/Assets.h"
#include "asset/Material.h"
#include "scene/ParticleSystem.h"
#include "scene/RenderContext.h"

struct ParticleDefinition {
	glm::vec3 dir = { 0.f, 0.f, 0.f };
	float radius = 0.f;
	float velocity = 0.f;
	float velocityVariation = 0.f;
	float size = 1.f;
	float sizeVariation = 1.f;
	Material* material = nullptr;
};

class ParticleGenerator
{
public:
	ParticleGenerator(
		const Assets& assets, 
		ParticleSystem* system,
		ParticleDefinition definition);

	virtual void generate(const RenderContext& ctx);

private:
	const Assets& assets;
	ParticleSystem* system;
	ParticleDefinition definition;
};
