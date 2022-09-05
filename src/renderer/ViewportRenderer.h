#pragma once

#include "Renderer.h"

class ViewportRenderer final : public Renderer
{
public:
	ViewportRenderer(const Assets& assets);

	void prepare() override;

	void update(const RenderContext& ctx, NodeRegistry& registry) override;
	void bind(const RenderContext& ctx) override;
	void render(const RenderContext& ctx, NodeRegistry& registry) override;
};

