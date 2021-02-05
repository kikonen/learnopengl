#pragma once

#include "Assets.h"
#include "Viewport.h"

class ViewportRenderer
{
public:
	ViewportRenderer(const Assets& assets);

	void prepare();

	void update(RenderContext& ctx, std::vector<Viewport*>& viewports);
	void bind(RenderContext& ctx);
	void render(RenderContext& ctx, std::vector<Viewport*>& viewports);

private:
	const Assets& assets;
};

