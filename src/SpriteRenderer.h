#pragma once

#include <vector>

#include "Renderer.h"
#include "RenderContext.h"
#include "Sprite.h"

class SpriteRenderer final : public Renderer
{
public:
	SpriteRenderer(const Assets& assets);

	void prepare();

	void update(RenderContext& ctx, std::map<NodeType*, std::vector<Sprite*>>& typeSprites);
	void bind(RenderContext& ctx, std::map<NodeType*, std::vector<Sprite*>>& typeSprites);
	void render(RenderContext& ctx, std::map<NodeType*, std::vector<Sprite*>>& typeSprites);
};
