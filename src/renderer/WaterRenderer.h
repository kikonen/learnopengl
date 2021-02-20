#pragma once

#include "Renderer.h"
#include "model/Water.h"

class WaterRenderer final : public Renderer
{
public:
	WaterRenderer(const Assets& assets);

	void prepare() override;

	void update(const RenderContext& ctx, NodeRegistry& registry) override;
	void bind(const RenderContext& ctx) override;
	void render(const RenderContext& ctx, NodeRegistry& registry) override;
};
