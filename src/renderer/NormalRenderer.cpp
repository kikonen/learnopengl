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

void NormalRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
	//for (auto& x : typeNodes) {
	//	for (auto& e : x.second) {
	//		e->update(ctx);
	//	}
	//}
}

void NormalRenderer::bind(const RenderContext& ctx)
{
}

void NormalRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.nodes) {
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

	for (auto& x : registry.terrains) {
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

	for (auto& x : registry.sprites) {
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
