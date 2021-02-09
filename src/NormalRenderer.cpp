#include "NormalRenderer.h"


NormalRenderer::NormalRenderer(const Assets& assets)
	: Renderer(assets)
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

void NormalRenderer::render(
	RenderContext& ctx,
	std::map<NodeType*, std::vector<Node*>>& typeNodes,
	std::map<NodeType*, std::vector<Sprite*>>& typeSprites,
	std::map<NodeType*, std::vector<Terrain*>>& typeTerrains)
{
	for (auto& x : typeNodes) {
		NodeType* t = x.first;
		if (t->light || t->skipShadow) continue;

		t->bind(ctx, normalShader);

		Batch& batch = t->batch;
		batch.bind(ctx, normalShader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, normalShader);
		}

		batch.flush(ctx, t);
	}

	for (auto& x : typeTerrains) {
		NodeType* t = x.first;
		if (t->light || t->skipShadow) continue;

		t->bind(ctx, normalShader);

		Batch& batch = t->batch;
		batch.bind(ctx, normalShader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, normalShader);
		}

		batch.flush(ctx, t);
	}

	for (auto& x : typeSprites) {
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
