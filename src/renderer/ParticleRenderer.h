#pragma once

#include "Renderer.h"

#include "render/TextureQuad.h"

class ParticleRenderer final : public Renderer
{
public:
    ParticleRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep),
        m_quad{ render::TextureQuad::get() }
    {}

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void render(
        const RenderContext& ctx);

private:
    Program* m_particleProgram{ nullptr };

    render::TextureQuad& m_quad;
};
