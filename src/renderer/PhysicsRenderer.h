#pragma once

#include "MeshRenderer.h"

class PhysicsRenderer : public MeshRenderer
{
public:
    virtual void prepareRT(const PrepareContext& ctx) override;

    virtual void render(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer) override;
};
