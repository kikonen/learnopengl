#pragma once

#include <vector>

#include "model/Particle.h"
#include "RenderContext.h"
#include "Batch.h"


class ParticleSystem final
{
public:
	ParticleSystem(const Assets& assets);

	void prepare();
	void update(RenderContext& ctx);
	void bind(RenderContext& ctx);
	void render(RenderContext& ctx);

	void addParticle(const Particle& particle);

private:
	const Assets& assets;
	std::vector<Particle> particles;

	Batch batch;

	Shader* particleShader = nullptr;
	NodeType* type;
};
