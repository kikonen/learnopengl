#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(const Assets& assets)
	: assets(assets)
{
}

void SpriteRenderer::prepare()
{
}

void SpriteRenderer::bind(RenderContext& ctx, std::vector<Sprite*>& sprites)
{
}

void SpriteRenderer::render(RenderContext& ctx, std::vector<Sprite*>& sprites)
{
	for (auto sprite : sprites) {
		sprite->bind(ctx, nullptr);
		sprite->draw(ctx);
	}
}
