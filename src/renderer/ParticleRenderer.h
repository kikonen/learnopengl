#pragma once

#include "Renderer.h"

class ParticleRenderer final : public Renderer
{
public:
    ParticleRenderer();

    virtual void prepare(
        const Assets& assets,
        ShaderRegistry& shaders,
        MaterialRegistry& materialRegistry) override;

    virtual void update(const RenderContext& ctx);

    void render(
        const RenderContext& ctx);

private:
    Shader* particleShader{ nullptr };
};

