#pragma once

#include "Renderer.h"

namespace render {
    class FrameBuffer;
}

class NormalRenderer final : public Renderer
{
public:
    NormalRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep) {}

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void render(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

private:
    void drawNodes(const RenderContext& ctx);

private:
    ki::program_id m_normalProgramId{ 0 };
};
