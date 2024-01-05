#pragma once

#include "Renderer.h"

namespace render {
    class FrameBuffer;
}

class ViewportRenderer final : public Renderer
{
public:
    ViewportRenderer(bool useFrameStep) : Renderer(useFrameStep) {}

    virtual void prepareRT(
        const Assets& assets,
        Registry* registry) override;

    void updateRT(const UpdateViewContext& ctx);

    void render(
        const RenderContext& ctx,
        render::FrameBuffer* destinationBuffer);
};
