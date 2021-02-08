#include "NormalRenderer.h"


NormalRenderer::NormalRenderer(const Assets& assets)
	: assets(assets)
{
	normalShader = Shader::getShader(assets, TEX_NORMAL);
	normalShader->prepare();
}

void NormalRenderer::prepare()
{
}

void NormalRenderer::update(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes)
{
	//for (auto& x : typeNodes) {
	//	for (auto& e : x.second) {
	//		e->update(ctx);
	//	}
	//}
}

void NormalRenderer::bind(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes)
{
}

void NormalRenderer::render(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes)
{
	for (auto& x : typeNodes) {
		x.first->bind(ctx, normalShader);

		for (auto& e : x.second) {
			e->bind(ctx, normalShader);
			e->draw(ctx);
		}
	}
}
