#pragma once

#include "Renderer.h"
#include "Water.h"

class WaterRenderer final : public Renderer
{
public:
	WaterRenderer(const Assets& assets);

	void prepare();
	void update(RenderContext& ctx, std::map<NodeType*, std::vector<Water*>>& typeNodes);
	void bind(RenderContext& ctx, std::map<NodeType*, std::vector<Water*>>& typeNodes);
	void render(RenderContext& ctx, std::map<NodeType*, std::vector<Water*>>& typeNodes);
};
