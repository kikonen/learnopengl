#pragma once

#include <vector>

#include "asset/Uniform.h"

#include "Renderer.h"

class ShadowCascade;
class Viewport;
class FrameBuffer;

class ShadowMapRenderer final : public Renderer
{
public:
    ShadowMapRenderer() {}
    virtual ~ShadowMapRenderer();

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void bindTexture(const RenderContext& ctx);

    void bind(const RenderContext& ctx);

    bool render(
        const RenderContext& ctx);

public:
    std::shared_ptr<Viewport> m_debugViewport;

private:
    // NOTE KI std::unique_ptr triggered exhaustive error loop
    std::vector<ShadowCascade*> m_cascades;

    Program* m_shadowDebugProgram{ nullptr };

    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.f;

    uniform::Float u_nearPlane{ "u_nearPlane", UNIFORM_NEAR_PLANE };
    uniform::Float u_farPlane{ "u_farPlane", UNIFORM_FAR_PLANE };
};
