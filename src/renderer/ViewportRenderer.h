#pragma once

#include "Renderer.h"

namespace render {
    class FrameBuffer;
}

class ViewportRenderer final : public Renderer
{
public:
    ViewportRenderer(bool useFrameStep) :
        Renderer("main", useFrameStep) {}

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(const UpdateViewContext& ctx);

    void render(
        const RenderContext& ctx,
        render::FrameBuffer* destinationBuffer);
};
