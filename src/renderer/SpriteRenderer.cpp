#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(const Assets& assets)
	: Renderer(assets)
{
}

void SpriteRenderer::prepare()
{
}

void SpriteRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
	for (auto& x : registry.sprites) {
		for (auto& e : x.second) {
			e->update(ctx);
		}
	}
}

void SpriteRenderer::bind(const RenderContext& ctx)
{
}

void SpriteRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	for (auto& x : registry.sprites) {
		NodeType* t = x.first;
		Shader* shader = t->bind(ctx, nullptr);
		if (!shader) continue;
		shader->reflectionMap.set(assets.reflectionMapUnitIndex);
		shader->shadowMap.set(assets.shadowMapUnitIndex);

		Batch& batch = t->batch;
		batch.bind(ctx, shader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, shader);
		}

		batch.flush(ctx, t);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}
