#pragma once

#include "Renderer.h"

class FrameBuffer;

class ViewportRenderer final : public Renderer
{
public:
    ViewportRenderer();

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    virtual void update(const RenderContext& ctx) override;

    void render(
        const RenderContext& ctx,
        FrameBuffer* destinationBuffer);
};

