#pragma once

#include "Renderer.h"

class ParticleRenderer final : public Renderer
{
public:
	ParticleRenderer(const Assets& assets);

	virtual void prepare(ShaderRegistry& shaders) override;
	virtual void update(const RenderContext& ctx, NodeRegistry& registry);
	virtual void bind(const RenderContext& ctx);
	virtual void render(const RenderContext& ctx, NodeRegistry& registry);

private:
	std::shared_ptr<Shader> particleShader;
};

