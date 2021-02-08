#pragma once

#include <vector>

#include "RenderContext.h"
#include "Node.h"

class NormalRenderer
{
public:
	NormalRenderer(const Assets& assets);

	void prepare();

	void update(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);
	void bind(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);
	void render(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);

private:
	const Assets& assets;
	Shader* normalShader = nullptr;

};

