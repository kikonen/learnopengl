#pragma once

#include "Renderer.h"
#include "scene/TextureBuffer.h"
#include "model/Node.h"

#include "model/Viewport.h"

class SkyboxRenderer;

class MirrorMapRenderer final : public Renderer
{
public:
    MirrorMapRenderer();
    ~MirrorMapRenderer();

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    void bindTexture(const RenderContext& ctx);

    void render(
        const RenderContext& ctx,
        const NodeRegistry& registry,
        SkyboxRenderer* skybox);

private:
    void drawNodes(const RenderContext& ctx, const NodeRegistry& registry, SkyboxRenderer* skybox, Node* current);
    Node* findClosest(const RenderContext& ctx, const NodeRegistry& registry);

public:
    std::shared_ptr<Viewport> debugViewport;

private:
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    std::unique_ptr<TextureBuffer> prev{ nullptr };
    std::unique_ptr<TextureBuffer> curr{ nullptr };
};
