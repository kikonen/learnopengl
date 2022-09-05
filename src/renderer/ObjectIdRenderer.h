#pragma once

#include "Renderer.h"

#include "scene/TextureBuffer.h"

class ObjectIdRenderer final : public Renderer
{
public:
	ObjectIdRenderer(const Assets& assets);
	virtual ~ObjectIdRenderer();

	int getObjectId(const RenderContext& ctx, double screenPosX, double screenPosY, Viewport* mainViewport);

	void prepare(ShaderRegistry& shaders) override;

	void update(const RenderContext& ctx, NodeRegistry& registry) override;
	void bind(const RenderContext& ctx) override;
	void render(const RenderContext& ctx, NodeRegistry& registry) override;

private:
	void drawNodes(const RenderContext& ctx, NodeRegistry& registry);

public:
	std::shared_ptr<Viewport> debugViewport;

private:
	std::shared_ptr<Shader> idShader;

	std::unique_ptr<TextureBuffer> idBuffer{ nullptr };
};

