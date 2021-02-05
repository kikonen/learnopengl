#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(const Assets& assets)
	: assets(assets)
{
}

void SpriteRenderer::prepare()
{
}

void SpriteRenderer::update(RenderContext& ctx, std::vector<Sprite*>& sprites)
{
	for (auto sprite : sprites) {
		sprite->update(ctx);
	}
}

void SpriteRenderer::bind(RenderContext& ctx, std::vector<Sprite*>& sprites)
{
}

void SpriteRenderer::render(RenderContext& ctx, std::vector<Sprite*>& sprites)
{
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	for (auto sprite : sprites) {
		sprite->bind(ctx, nullptr);
		sprite->draw(ctx);
	}

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}
