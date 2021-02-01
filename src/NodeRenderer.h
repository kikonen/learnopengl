#pragma once

#include <vector>

#include "RenderContext.h"
#include "Node.h"

class NodeRenderer
{
public:
	NodeRenderer(const Assets& assets);

	void prepare();
	void bind(RenderContext& ctx, std::vector<Node*>& nodes);
	void render(RenderContext& ctx, std::vector<Node*>& nodes);

private:
	int drawNodes(RenderContext& ctx, std::vector<Node*>& nodes, bool selection);
	void drawBlended(RenderContext& ctx, std::vector<Node*>& nodes);

	void drawSelectionStencil(RenderContext& ctx, std::vector<Node*>& nodes);

private:
	const Assets& assets;

	Shader* selectionShader = nullptr;

};

