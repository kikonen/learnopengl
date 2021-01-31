#include "NormalRenderer.h"


NormalRenderer::NormalRenderer(const Assets& assets)
	: assets(assets)
{
	shader = Shader::getShader(assets, TEX_NORMAL, "");
}

void NormalRenderer::prepare(std::vector<Node*>& nodes)
{
	for (auto node : nodes) {
		node->prepare(shader);
	}
}

void NormalRenderer::bind(RenderContext& ctx, std::vector<Node*>& nodes)
{
}

void NormalRenderer::render(RenderContext& ctx, std::vector<Node*>& nodes)
{
	for (auto node : nodes) {
		node->bind(ctx, shader);
		node->draw(ctx);
	}
}
