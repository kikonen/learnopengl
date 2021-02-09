#pragma once

#include <vector>

#include "Renderer.h"
#include "RenderContext.h"
#include "Node.h"

class NormalRenderer final : public Renderer
{
public:
	NormalRenderer(const Assets& assets);

	void prepare();

	void update(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);
	void bind(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);
	void render(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);

private:
	Shader* normalShader = nullptr;
};

