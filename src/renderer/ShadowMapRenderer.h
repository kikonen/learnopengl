#pragma once

#include <vector>

#include "Renderer.h"
#include "model/Viewport.h"
#include "scene/ShadowBuffer.h"


// NOTE KI MUST match lookup() in light shadow shader
const unsigned int SHADOW_WIDTH = 1000,
	SHADOW_HEIGHT = 1000;


class ShadowMapRenderer final : public Renderer
{
public:
	ShadowMapRenderer(const Assets& assets);

	void prepare() override;

	void bindTexture(const RenderContext& ctx);

	void bind(const RenderContext& ctx) override;
	void render(const RenderContext& ctx, NodeRegistry& registry) override;

private:
	void drawNodes(const RenderContext& ctx, NodeRegistry& registry);

public:
	ShadowBuffer shadowBuffer = { SHADOW_WIDTH, SHADOW_HEIGHT };

	Viewport* debugViewport = nullptr;

private:
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	int drawIndex = 0;
	int drawSkip = 1;

	bool rendered = false;

	Shader* shadowShader = nullptr;
	Shader* shadowDebugShader = nullptr;
};

