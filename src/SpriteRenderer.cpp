#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(const Assets& assets)
	: assets(assets)
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
		Shader* shader = x.first->bind(ctx, nullptr);
		if (!shader) continue;
		shader->shadowMap.set(assets.shadowMapUnitIndex);

		for (auto& e : x.second) {
			e->bind(ctx, shader);
			e->draw(ctx);
		}
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}
