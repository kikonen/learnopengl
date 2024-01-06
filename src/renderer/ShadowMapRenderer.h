#pragma once

#include <vector>

#include "Renderer.h"

namespace render {
    class FrameBuffer;
}

class ShadowCascade;
class Viewport;

class ShadowMapRenderer final : public Renderer
{
public:
    ShadowMapRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep)
    {}

    virtual ~ShadowMapRenderer();

    virtual void prepareRT(
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

    int m_activeCascade{ 0 };

    float m_rotateElapsedSecs{ 0.f };

    std::vector<float> m_planes;
    std::vector<int> m_mapSizes;
};
