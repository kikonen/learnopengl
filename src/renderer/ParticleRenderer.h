#pragma once

#include "Renderer.h"

class ParticleRenderer : public Renderer
{
public:
	ParticleRenderer(const Assets& assets);

	virtual void prepare();
	virtual void update(const RenderContext& ctx, NodeRegistry& registry);
	virtual void bind(const RenderContext& ctx);
	virtual void render(const RenderContext& ctx, NodeRegistry& registry);

private:
	Shader* particleShader;
};

