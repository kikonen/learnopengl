#pragma once

#include "Renderer.h"
#include "TextureBuffer.h"

#include "NodeRegistry.h"

const unsigned int REFLECTION_WIDTH = 640,
  REFLECTION_HEIGHT = 480;

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

	int drawIndex = 0;
	int drawSkip = 4;

	unsigned int cubeMapTextureID;

	TextureBuffer textureBuffers[6] = {
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
		{ REFLECTION_WIDTH, REFLECTION_HEIGHT },
	};
};
