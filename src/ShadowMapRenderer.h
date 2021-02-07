#pragma once

#include <vector>

#include "RenderContext.h"
#include "Node.h"
#include "Sprite.h"
#include "Terrain.h"
#include "FrameBuffer.h"
#include "Viewport.h"

// NOTE KI MUST match lookup() in light shadow shader
const unsigned int SHADOW_WIDTH = 1000,
	SHADOW_HEIGHT = 1000;


class ShadowMapRenderer
{
public:
	ShadowMapRenderer(const Assets& assets);

	void prepare();

	void bindTexture(RenderContext& ctx);

	void bind(RenderContext& ctx);
	void render(
		RenderContext& ctx,
		std::map<int, std::vector<Node*>>& typeNodes,
		std::map<int, std::vector<Sprite*>>& typeSprites,
		std::map<int, std::vector<Terrain*>>& typeTerrains);

private:
	void drawNodes(
		RenderContext& ctx,
		std::map<int, std::vector<Node*>>& typeNodes,
		std::map<int, std::vector<Sprite*>>& typeSprites,
		std::map<int, std::vector<Terrain*>>& typeTerrains);

public:
	FrameBuffer frameBuffer = { SHADOW_WIDTH, SHADOW_HEIGHT };

	Viewport* debugViewport = nullptr;

private:
	const Assets& assets;

	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	Shader* shadowShader = nullptr;
	Shader* shadowDebugShader = nullptr;
};

