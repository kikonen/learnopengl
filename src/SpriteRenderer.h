#pragma once

#include <vector>

#include "RenderContext.h"
#include "Sprite.h"

class SpriteRenderer
{
public:
	SpriteRenderer(const Assets& assets);

	void prepare();

	void update(RenderContext& ctx, std::vector<Sprite*>& sprites);
	void bind(RenderContext& ctx, std::vector<Sprite*>& sprites);
	void render(RenderContext& ctx, std::vector<Sprite*>& sprites);

private:
	const Assets& assets;
};

