#pragma once

#include <vector>

#include "RenderContext.h"
#include "Node.h"
#include "FrameBuffer.h"
#include "Viewport.h"

const unsigned int SHADOW_WIDTH = 1024,
	SHADOW_HEIGHT = 1024;


class ShadowMapRenderer
{
public:
	ShadowMapRenderer(const Assets& assets);

	void prepare();
	void bind(RenderContext& ctx);
	void bindTexture(RenderContext& ctx);
	void render(RenderContext& ctx, std::vector<Node*>& nodes);

private:
	void drawNodes(RenderContext& ctx, std::vector<Node*>& nodes);
	void drawBlendedNodes(std::vector<Node*>& nodes, RenderContext& ctx);

public:
	FrameBuffer frameBuffer = { SHADOW_WIDTH, SHADOW_HEIGHT };

	Viewport* debugViewport;

private:
	const Assets& assets;

	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	Shader* shadowShader = nullptr;
	Shader* shadowDebugShader = nullptr;
};

