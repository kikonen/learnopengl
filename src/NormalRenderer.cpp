#include "NormalRenderer.h"


NormalRenderer::NormalRenderer(const Assets& assets)
	: assets(assets)
{
	shader = Shader::getShader(assets, TEX_NORMAL);
	shader->prepare();
}

void NormalRenderer::prepare()
{
}

void NormalRenderer::update(RenderContext& ctx, std::map<int, std::vector<Node*>>& typeNodes)
{
	//for (auto& x : typeNodes) {
	//	for (auto& e : x.second) {
	//		e->update(ctx);
	//	}
	//}
}

void NormalRenderer::bind(RenderContext& ctx, std::map<int, std::vector<Node*>>& typeNodes)
{
}

void NormalRenderer::render(RenderContext& ctx, std::map<int, std::vector<Node*>>& typeNodes)
{
	for (auto& x : typeNodes) {
		for (auto& e : x.second) {
			e->bind(ctx, shader);
			e->draw(ctx);
		}
	}
}
