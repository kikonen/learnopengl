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
		std::vector<Node*>& nodes,
		std::vector<Sprite*>& sprites,
		std::vector<Terrain*>& terrains);

private:
	void drawNodes(
		RenderContext& ctx,
		std::vector<Node*>& nodes,
		std::vector<Sprite*>& sprites,
		std::vector<Terrain*>& terrains);

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

