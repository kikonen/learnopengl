#pragma once

#include "Renderer.h"

#include "scene/TextureBuffer.h"

class ObjectIdRenderer final : public Renderer
{
public:
	ObjectIdRenderer(const Assets& assets);

	int getObjectId(const RenderContext& ctx, double screenPosX, double screenPosY, Viewport* mainViewport);

	void prepare() override;

	void update(const RenderContext& ctx, NodeRegistry& registry) override;
	void bind(const RenderContext& ctx) override;
	void render(const RenderContext& ctx, NodeRegistry& registry) override;

private:
	void drawNodes(const RenderContext& ctx, NodeRegistry& registry);

public:
	Viewport* debugViewport = nullptr;

private:
	std::shared_ptr<Shader> idShader = nullptr;

	TextureBuffer* idBuffer = nullptr;
};

