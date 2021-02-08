#pragma once

#include <vector>

#include "RenderContext.h"
#include "Sprite.h"

class SpriteRenderer
{
public:
	SpriteRenderer(const Assets& assets);

	void prepare();

	void update(RenderContext& ctx, std::map<NodeType*, std::vector<Sprite*>>& typeSprites);
	void bind(RenderContext& ctx, std::map<NodeType*, std::vector<Sprite*>>& typeSprites);
	void render(RenderContext& ctx, std::map<NodeType*, std::vector<Sprite*>>& typeSprites);

private:
	const Assets& assets;
};

