#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(const Assets& assets)
	: Renderer(assets)
{
}

void SpriteRenderer::prepare()
{
}

void SpriteRenderer::update(RenderContext& ctx, std::map<NodeType*, std::vector<Sprite*>>& typeSprites)
{
	for (auto& x : typeSprites) {
		for (auto& e : x.second) {
			e->update(ctx);
		}
	}
}

void SpriteRenderer::bind(RenderContext& ctx, std::map<NodeType*, std::vector<Sprite*>>& typeSprites)
{
}

void SpriteRenderer::render(RenderContext& ctx, std::map<NodeType*, std::vector<Sprite*>>& typeSprites)
{
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	for (auto& x : typeSprites) {
		NodeType* t = x.first;
		Shader* shader = t->bind(ctx, nullptr);
		if (!shader) continue;
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
