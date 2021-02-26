#pragma once

#include "Renderer.h"
#include "asset/DynamicCubeMap.h"
#include "model/Water.h"

class SkyboxRenderer;

class WaterRenderer final : public Renderer
{
public:
	WaterRenderer(const Assets& assets);
	~WaterRenderer();

	void prepare() override;

	void update(const RenderContext& ctx, NodeRegistry& registry) override;

	void bindTexture(const RenderContext& ctx);

	void bind(const RenderContext& ctx);
	void render(const RenderContext& ctx, NodeRegistry& registry) override;

public:

private:
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	int drawIndex = 0;
	int drawSkip = 1;

	DynamicCubeMap* reflectionMap = nullptr;
	DynamicCubeMap* refractionMap = nullptr;

	bool rendered = false;
};
