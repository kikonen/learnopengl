#pragma once

#include <vector>

#include "Renderer.h"
#include "model/Viewport.h"
#include "scene/ShadowBuffer.h"


class ShadowMapRenderer final : public Renderer
{
public:
    ShadowMapRenderer(const Assets& assets);
    virtual ~ShadowMapRenderer();

    void prepare(ShaderRegistry& shaders) override;

    void bindTexture(const RenderContext& ctx);

    void bind(const RenderContext& ctx) override;
    void render(const RenderContext& ctx, NodeRegistry& registry) override;

private:
    void drawNodes(const RenderContext& ctx, NodeRegistry& registry);

public:
    std::unique_ptr<ShadowBuffer> shadowBuffer{ nullptr };

    std::shared_ptr<Viewport> debugViewport;

private:
    std::shared_ptr<Shader> shadowShader{ nullptr };
    std::shared_ptr<Shader> shadowDebugShader{ nullptr };
};

