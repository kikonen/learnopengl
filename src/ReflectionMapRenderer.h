#pragma once

#include "Renderer.h"
#include "FrameBuffer.h"

#include "NodeRegistry.h"
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
	void bind(const RenderContext& ctx);
	void bindTexture(const RenderContext& ctx);
	void render(const RenderContext& ctx, NodeRegistry& registry);

private:
	void drawNodes(const RenderContext& ctx, NodeRegistry& registry);

public:
	glm::vec3 center = { 0, 0, 0 };

private:
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	FrameBuffer frameBuffers[6] = {
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
	};
};
