#pragma once

#include "Renderer.h"
#include "scene/TextureBuffer.h"
#include "model/Water.h"

#include "model/Viewport.h";

class SkyboxRenderer;

class WaterMapRenderer final : public Renderer
{
public:
	WaterMapRenderer(const Assets& assets);
	~WaterMapRenderer();

	void prepare() override;

	void bindTexture(const RenderContext& ctx);

	void bind(const RenderContext& ctx);
	void render(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox);

private:
	void drawNodes(const RenderContext& ctx, NodeRegistry& registry);
	Water* findClosest(const RenderContext& ctx, NodeRegistry& registry);

public:
	Viewport* debugViewport = nullptr;

private:
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	int drawIndex = 0;
	int drawSkip = 2;

	TextureBuffer* reflectionBuffer = nullptr;
	TextureBuffer* refractionBuffer = nullptr;

	bool rendered = false;
};
