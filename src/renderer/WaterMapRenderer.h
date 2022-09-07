#pragma once

#include "Renderer.h"
#include "scene/TextureBuffer.h"
#include "model/Water.h"

#include "model/Viewport.h"

class SkyboxRenderer;

class WaterMapRenderer final : public Renderer
{
public:
    WaterMapRenderer(const Assets& assets);
    virtual ~WaterMapRenderer();

    void prepare(ShaderRegistry& shaders) override;

    void bindTexture(const RenderContext& ctx);

    void bind(const RenderContext& ctx);
    void render(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox);

private:
    void drawNodes(const RenderContext& ctx, NodeRegistry& registry, SkyboxRenderer* skybox, Node* current);
    Water* findClosest(const RenderContext& ctx, NodeRegistry& registry);

public:
    std::shared_ptr<Viewport> reflectionDebugViewport;
    std::shared_ptr<Viewport> refractionDebugViewport;

private:
    float nearPlane{ 0.1f };
    float farPlane{ 1000.0f };

    std::unique_ptr<TextureBuffer> reflectionBuffer{ nullptr };
    std::unique_ptr<TextureBuffer> refractionBuffer{ nullptr };

    unsigned int noiseTextureID{ 0 };
};
