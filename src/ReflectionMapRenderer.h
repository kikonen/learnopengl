#pragma once

#include "Renderer.h"
#include "FrameBuffer.h"

#include "Sprite.h"
#include "Terrain.h"

const unsigned int REFLECTION_WIDTH = 1000,
	REFLECTION_HEIGHT = 1000;

class ReflectionMapRenderer final : public Renderer
{
public:
	ReflectionMapRenderer(const Assets& assets);
	~ReflectionMapRenderer();

	void prepare();
	void bind(RenderContext& ctx);
	void bindTexture(RenderContext& ctx);
	void render(
		RenderContext& ctx,
		std::map<NodeType*, std::vector<Node*>>& typeNodes,
		std::map<NodeType*, std::vector<Sprite*>>& typeSprites,
		std::map<NodeType*, std::vector<Terrain*>>& typeTerrains);

private:
	void drawNodes(
		RenderContext& ctx,
		std::map<NodeType*, std::vector<Node*>>& typeNodes,
		std::map<NodeType*, std::vector<Sprite*>>& typeSprites,
		std::map<NodeType*, std::vector<Terrain*>>& typeTerrains);

public:
	glm::vec3 center = { 0, 0, 0 };

private:
	FrameBuffer frameBuffers[6] = {
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
	};
};
