#pragma once

#include <vector>

#include "Renderer.h"

class NormalRenderer final : public Renderer
{
public:
	NormalRenderer(const Assets& assets);

	void prepare() override;

	void update(const RenderContext& ctx, NodeRegistry& registry) override;
	void bind(const RenderContext& ctx) override;
	void render(const RenderContext& ctx, NodeRegistry& registry) override;

private:
	Shader* normalShader = nullptr;
};

