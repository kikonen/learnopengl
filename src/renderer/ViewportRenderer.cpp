#include "ViewportRenderer.h"

ViewportRenderer::ViewportRenderer(const Assets& assets)
	: Renderer(assets)
{
}

void ViewportRenderer::prepare()
{
}

void ViewportRenderer::bind(const RenderContext& ctx)
{
}

void ViewportRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto viewport : registry.viewports) {
		viewport->update(ctx);
	}
}

void ViewportRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto viewport : registry.viewports) {
		viewport->bind(ctx);
		viewport->draw(ctx);
	}
	glBindVertexArray(0);
}
