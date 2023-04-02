#pragma once

#include <vector>

#include "asset/Uniform.h"

#include "Renderer.h"

class Viewport;
class FrameBuffer;


class ShadowMapRenderer final : public Renderer
{
public:
    ShadowMapRenderer() {}
    virtual ~ShadowMapRenderer() = default;

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void bindTexture(const RenderContext& ctx);

    void bind(const RenderContext& ctx);

    bool render(
        const RenderContext& ctx);

private:
    void drawNodes(const RenderContext& ctx);

public:
    std::unique_ptr<FrameBuffer> m_shadowBuffer{ nullptr };

    std::shared_ptr<Viewport> m_debugViewport;

private:
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.f;
    float m_frustumSize = 100.f;

    //Program* m_shadowProgram{ nullptr };
    Program* m_solidShadowProgram{ nullptr };
    Program* m_blendedShadowProgram{ nullptr };
    Program* m_shadowDebugProgram{ nullptr };

    uniform::Float u_nearPlane{ "u_nearPlane", UNIFORM_NEAR_PLANE };
    uniform::Float u_farPlane{ "u_farPlane", UNIFORM_FAR_PLANE };
};
