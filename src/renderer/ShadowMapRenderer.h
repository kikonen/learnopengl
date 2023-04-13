#pragma once

#include <vector>

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

    int m_activeCascade;

    Program* m_shadowDebugProgram{ nullptr };

    std::vector<float> m_planes;
    std::vector<int> m_mapSizes;
};
