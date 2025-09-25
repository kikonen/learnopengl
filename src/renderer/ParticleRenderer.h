#pragma once

#include "Renderer.h"

class Program;

class ParticleRenderer final : public Renderer
{
public:
    ParticleRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep)
    {}

    virtual void prepareRT(
        const PrepareContext& ctx) override;

    void render(
        const render::RenderContext& ctx);

private:
    Program* m_particleProgram{ nullptr };
};
