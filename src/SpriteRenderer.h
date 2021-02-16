#pragma once

#include <vector>

#include "Renderer.h"
#include "RenderContext.h"
#include "Sprite.h"
#include "NodeRegistry.h"

class SpriteRenderer final : public Renderer
{
public:
	SpriteRenderer(const Assets& assets);

	void prepare();

	void update(const RenderContext& ctx, NodeRegistry& registry);
	void bind(const RenderContext& ctx, NodeRegistry& registry);
	void render(const RenderContext& ctx, NodeRegistry& registry);
};
