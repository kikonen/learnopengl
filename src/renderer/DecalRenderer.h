#pragma once

#include "Renderer.h"

class DecalRenderer final : public Renderer
{
public:
    DecalRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep)
    {}

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void renderSolid(
        const RenderContext& ctx);

    void renderBlend(
        const RenderContext& ctx);

private:
    ki::program_id m_alphaDecalProgramId{ 0 };
    ki::program_id m_blendDecalProgramId{ 0 };
    ki::program_id m_solidDecalProgramId{ 0 };
};
