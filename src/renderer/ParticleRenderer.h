#pragma once

#include "Renderer.h"

class ParticleRenderer final : public Renderer
{
public:
    ParticleRenderer();

    virtual void prepare(
        const Assets& assets,
        Registry* registry) override;

    void render(
        const RenderContext& ctx);

private:
    Program* particleProgram{ nullptr };
};
