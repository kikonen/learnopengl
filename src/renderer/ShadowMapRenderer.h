#pragma once

#include <vector>
#include <memory>

#include "Renderer.h"

namespace kigl {
    class GLState;
}

namespace render {
    class FrameBuffer;
}

class ShadowCascade;
class Viewport;
struct DataUBO;

class ShadowMapRenderer final : public Renderer
{
public:
    ShadowMapRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep)
    {}

    virtual ~ShadowMapRenderer();

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void bindTexture(kigl::GLState& state);

    void bind(
        const RenderContext& ctx,
        DataUBO& dataUbo);

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
