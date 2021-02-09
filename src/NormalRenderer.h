#pragma once

#include <vector>

#include "Renderer.h"
#include "RenderContext.h"
#include "Node.h"
#include "Sprite.h"
#include "Terrain.h"

class NormalRenderer final : public Renderer
{
public:
	NormalRenderer(const Assets& assets);

	void prepare();

	void update(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);
	void bind(RenderContext& ctx, std::map<NodeType*, std::vector<Node*>>& typeNodes);
	void render(
		RenderContext& ctx,
		std::map<NodeType*, std::vector<Node*>>& typeNodes,
		std::map<NodeType*, std::vector<Sprite*>>& typeSprites,
		std::map<NodeType*, std::vector<Terrain*>>& typeTerrains);

private:
	Shader* normalShader = nullptr;
};

