#pragma once

#include "Renderer.h"

#include "render/TextureQuad.h"

namespace mesh {
    class Mesh;
}


class DecalRenderer final : public Renderer
{
public:
    DecalRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep)
    {}

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void render(
        const RenderContext& ctx);

private:
    ki::program_id m_alphaDecalProgramId{ 0 };
    ki::program_id m_solidDecalProgramId{ 0 };

    std::unique_ptr<mesh::Mesh> m_quad;
};
