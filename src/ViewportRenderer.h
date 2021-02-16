#pragma once

#include "Renderer.h"
#include "Assets.h"
#include "Viewport.h"
#include "NodeRegistry.h"

class ViewportRenderer : public Renderer
{
public:
	ViewportRenderer(const Assets& assets);

	void prepare();

	void update(const RenderContext& ctx, NodeRegistry& registry);
	void bind(const RenderContext& ctx);
	void render(const RenderContext& ctx, NodeRegistry& registry);
};

