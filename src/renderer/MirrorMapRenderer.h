#pragma once

#include "Renderer.h"
#include "scene/TextureBuffer.h"
#include "model/Node.h"

#include "model/Viewport.h"

class SkyboxRenderer;

class MirrorMapRenderer final : public Renderer
{
public:
	MirrorMapRenderer(const Assets& assets);
	~MirrorMapRenderer();

	void prepare() override;

	void bindTexture(const RenderContext& ctx);

	void bind(const RenderContext& ctx);
	void render(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox);

private:
	void drawNodes(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox, Node* current);
	Node* findClosest(const RenderContext& ctx, NodeRegistry& registry);

public:
	std::shared_ptr<Viewport> debugViewport;

private:
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	TextureBuffer* reflectionBuffer = nullptr;
};
