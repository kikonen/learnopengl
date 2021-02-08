#pragma once

#include <vector>

#include "RenderContext.h"
#include "Node.h"

class NodeRenderer
{
public:
	NodeRenderer(const Assets& assets);

	void prepare();
	void update(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);
	void bind(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);
	void render(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);

private:
	int drawNodes(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes, bool selection);
	void drawBlended(RenderContext& ctx, std::vector<Node*>& nodes);

	void drawSelectionStencil(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);

private:
	const Assets& assets;

	Shader* selectionShader = nullptr;

};

