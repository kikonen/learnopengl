#include "NormalRenderer.h"


NormalRenderer::NormalRenderer(const Assets& assets)
	: Renderer(assets)
{
	normalShader = Shader::getShader(assets, TEX_NORMAL);
}

void NormalRenderer::prepare()
{
	normalShader->prepare();
}

void NormalRenderer::bind(const RenderContext& ctx)
{
}

void NormalRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	drawNodes(ctx, registry);
}

void NormalRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.nodes) {
		NodeType* t = x.first;
		t->bind(ctx, normalShader);

		Batch& batch = t->batch;
		batch.bind(ctx, normalShader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, normalShader);
		}

		batch.flush(ctx, t);
	}
}
