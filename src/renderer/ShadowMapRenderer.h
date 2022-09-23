#pragma once

#include <vector>

#include "Renderer.h"
#include "model/Viewport.h"
#include "scene/ShadowBuffer.h"


class ShadowMapRenderer final : public Renderer
{
public:
    ShadowMapRenderer();
    virtual ~ShadowMapRenderer();

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    void bindTexture(const RenderContext& ctx);

    virtual void bind(const RenderContext& ctx) override;

    void render(
        const RenderContext& ctx,
        const NodeRegistry& registry);

private:
    void drawNodes(const RenderContext& ctx, const NodeRegistry& registry);

public:
    std::unique_ptr<ShadowBuffer> shadowBuffer{ nullptr };

    std::shared_ptr<Viewport> debugViewport;

private:
    Shader* solidShadowShader{ nullptr };
    Shader* blendedShadowShader{ nullptr };
    Shader* shadowDebugShader{ nullptr };
};

