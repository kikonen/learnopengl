#include "NormalRenderer.h"


NormalRenderer::NormalRenderer(const Assets& assets)
	: assets(assets)
{
	shader = Shader::getShader(assets, TEX_NORMAL, "");
	shader->prepare();
}

void NormalRenderer::prepare()
{
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
