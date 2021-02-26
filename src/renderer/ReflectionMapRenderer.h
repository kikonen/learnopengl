#pragma once

#include "Renderer.h"

#include "asset/DynamicCubeMap.h"
#include "scene/TextureBuffer.h"

class SkyboxRenderer;

class ReflectionMapRenderer final : public Renderer
{
public:
	ReflectionMapRenderer(const Assets& assets);
	~ReflectionMapRenderer();

	void prepare();
	void bind(const RenderContext& ctx);
	void bindTexture(const RenderContext& ctx);
	void render(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox);

private:
	void drawNodes(const RenderContext& ctx, NodeRegistry& registry);
	Node* findCenter(const RenderContext& ctx, NodeRegistry& registry);

public:
	glm::vec3 center = { 0, 0, 0 };

private:
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	int drawIndex = 0;
	int drawSkip = 1;

	DynamicCubeMap* reflectionMap = nullptr;

	bool rendered = false;
};
