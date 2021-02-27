#pragma once

#include "Renderer.h"
#include "scene/TextureBuffer.h"
#include "model/Water.h"

class SkyboxRenderer;

class WaterMapRenderer final : public Renderer
{
public:
	WaterMapRenderer(const Assets& assets);
	~WaterMapRenderer();

	void prepare() override;

	void bindTexture(const RenderContext& ctx);

	void bind(const RenderContext& ctx);
	void render(const RenderContext& ctx, NodeRegistry& registry) override;

public:

private:
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	int drawIndex = 0;
	int drawSkip = 1;

	TextureBuffer* reflectionMap = nullptr;
	TextureBuffer* refractionMap = nullptr;

	bool rendered = false;
};
