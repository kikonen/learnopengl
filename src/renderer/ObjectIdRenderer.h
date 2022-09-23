#pragma once

#include "Renderer.h"

#include "scene/TextureBuffer.h"

class ObjectIdRenderer final : public Renderer
{
public:
    ObjectIdRenderer();
    virtual ~ObjectIdRenderer();

    int getObjectId(const RenderContext& ctx, double screenPosX, double screenPosY, Viewport* mainViewport);

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    virtual void update(const RenderContext& ctx, const NodeRegistry& registry) override;
    virtual void bind(const RenderContext& ctx) override;

    void render(
        const RenderContext& ctx,
        const NodeRegistry& registry);

private:
    void drawNodes(const RenderContext& ctx, const NodeRegistry& registry);

public:
    std::shared_ptr<Viewport> debugViewport;

private:
    Shader* idShader{ nullptr };

    std::unique_ptr<TextureBuffer> idBuffer{ nullptr };
};

