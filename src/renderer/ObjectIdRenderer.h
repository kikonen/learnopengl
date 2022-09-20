#pragma once

#include "Renderer.h"

#include "scene/TextureBuffer.h"

class ObjectIdRenderer final : public Renderer
{
public:
    ObjectIdRenderer();
    virtual ~ObjectIdRenderer();

    int getObjectId(const RenderContext& ctx, double screenPosX, double screenPosY, Viewport* mainViewport);

    void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    void update(const RenderContext& ctx, const NodeRegistry& registry) override;
    void bind(const RenderContext& ctx) override;
    void render(const RenderContext& ctx, const NodeRegistry& registry) override;

private:
    void drawNodes(const RenderContext& ctx, const NodeRegistry& registry);

public:
    std::shared_ptr<Viewport> debugViewport;

private:
    std::shared_ptr<Shader> idShader;

    std::unique_ptr<TextureBuffer> idBuffer{ nullptr };
};

