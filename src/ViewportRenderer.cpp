#include "ViewportRenderer.h"

ViewportRenderer::ViewportRenderer(const Assets& assets)
	: assets(assets)
{
}

void ViewportRenderer::prepare()
{
}

void ViewportRenderer::bind(RenderContext& ctx)
{
}

void ViewportRenderer::render(RenderContext& ctx, std::vector<Viewport*>& viewports)
{
	for (auto viewport : viewports) {
		viewport->draw(ctx);
	}
}
