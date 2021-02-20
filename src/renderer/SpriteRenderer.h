#pragma once

#include <vector>

#include "Renderer.h"

class SpriteRenderer final : public Renderer
{
public:
	SpriteRenderer(const Assets& assets);

	void prepare() override;

	void update(const RenderContext& ctx, NodeRegistry& registry) override;
	void bind(const RenderContext& ctx) override;
	void render(const RenderContext& ctx, NodeRegistry& registry) override;
};
