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
		batch.matrices.clear();

		for (auto& e : x.second) {
			if (batch.size) {
				e->bindBatch(ctx, batch);
				if (batch.matrices.size() == batch.size) {
					batch.update(batch.matrices.size());
					t->mesh->drawInstanced(ctx, batch.matrices.size());
					batch.matrices.clear();
				}
			}
			else {
				e->bind(ctx, shader);
				e->draw(ctx);
			}
		}

		if (!batch.matrices.empty()) {
			batch.update(batch.matrices.size());
			t->mesh->drawInstanced(ctx, batch.matrices.size());
		}
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}
