#pragma once

#include "Renderer.h"

namespace decal {
    class DecalCollection;
}

class DecalRenderer final : public Renderer
{
public:
    DecalRenderer(bool useFrameStep);

    ~DecalRenderer();

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void renderSolid(
        const render::RenderContext& ctx);

    void renderBlend(
        const render::RenderContext& ctx);

private:
    void renderSolidCollection(
        const render::RenderContext& ctx,
        const decal::DecalCollection& collection);

    void renderBlendCollection(
        const render::RenderContext& ctx,
        const decal::DecalCollection& collection);

private:
    ki::program_id m_alphaDecalProgramId{ 0 };
    ki::program_id m_blendDecalProgramId{ 0 };
    ki::program_id m_solidDecalProgramId{ 0 };
};
