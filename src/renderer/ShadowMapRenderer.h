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

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void bindTexture(const RenderContext& ctx);

    void bind(const RenderContext& ctx);

    void render(
        const RenderContext& ctx);

private:
    void drawNodes(const RenderContext& ctx);

public:
    std::unique_ptr<ShadowBuffer> m_shadowBuffer{ nullptr };

    std::shared_ptr<Viewport> m_debugViewport;

private:
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.f;
    float m_frustumSize = 100.f;

    Shader* m_shadowShader{ nullptr };
    //Shader* m_solidShadowShader{ nullptr };
    //Shader* m_blendedShadowShader{ nullptr };
    Shader* m_shadowDebugShader{ nullptr };
};

