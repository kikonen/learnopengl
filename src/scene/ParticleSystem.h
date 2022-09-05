#pragma once

#include <vector>

#include "asset/ShaderRegistry.h"

#include "model/Particle.h"
#include "RenderContext.h"
#include "Batch.h"


class ParticleSystem final
{
public:
	ParticleSystem(const Assets& assets);

	void prepare(ShaderRegistry& shaders);
	void update(RenderContext& ctx);
	void bind(RenderContext& ctx);
	void render(RenderContext& ctx);

	void addParticle(const Particle& particle);

public:
	const Assets& assets;

private:
	std::vector<Particle> particles;

	Batch batch;

	std::shared_ptr<Shader> particleShader = nullptr;
	NodeType* type;
};
