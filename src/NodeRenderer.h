#pragma once

#include <vector>

#include "Renderer.h"
#include "RenderContext.h"
#include "Node.h"
#include "NodeRegistry.h"

class NodeRenderer final : public Renderer
{
public:
	NodeRenderer(const Assets& assets);

	void prepare();
	void update(const RenderContext& ctx, NodeRegistry& registry);
	void bind(const RenderContext& ctx, NodeRegistry& registry);
	void render(const RenderContext& ctx, NodeRegistry& registry);

private:
	int drawNodes(const RenderContext& ctx, NodeRegistry& registry, bool selection);
	void drawBlended(const RenderContext& ctx, std::vector<Node*>& nodes);

	void drawSelectionStencil(const RenderContext& ctx, NodeRegistry& registry);

private:
	Shader* selectionShader = nullptr;
};
