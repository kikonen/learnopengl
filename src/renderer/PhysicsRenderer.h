#pragma once

#include "MeshRenderer.h"

class PhysicsRenderer : public MeshRenderer
{
public:
    virtual void render(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer) override;
};
