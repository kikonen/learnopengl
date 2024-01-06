#pragma once

#include "Renderer.h"

class ParticleRenderer final : public Renderer
{
public:
    ParticleRenderer(bool useFrameStep)
        : Renderer("main", useFrameStep)
    {}

    virtual void prepareRT(
        const Assets& assets,
        Registry* registry) override;

    void render(
        const RenderContext& ctx);

private:
    Program* particleProgram{ nullptr };
};
