#pragma once

#include "Renderer.h"
#include "FrameBuffer.h"

#include "Sprite.h"
#include "Terrain.h"

const unsigned int REFLECTION_WIDTH = 1000,
	REFLECTION_HEIGHT = 1000;

class ReflectionMapRenderer final : public Renderer
{
	ReflectionMapRenderer(const Assets& assets);
	~ReflectionMapRenderer();

	void bind(RenderContext& ctx);
	void render(
		RenderContext& ctx,
		std::map<NodeType*, std::vector<Node*>>& typeNodes,
		std::map<NodeType*, std::vector<Sprite*>>& typeSprites,
		std::map<NodeType*, std::vector<Terrain*>>& typeTerrains);

private:
	FrameBuffer frameBuffer = { REFLECTION_WIDTH, REFLECTION_HEIGHT };
};

