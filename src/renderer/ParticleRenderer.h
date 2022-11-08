#pragma once

#include "Renderer.h"

class ParticleRenderer final : public Renderer
{
public:
    ParticleRenderer();

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders) override;
    virtual void update(const RenderContext& ctx, const NodeRegistry& registry);

    void render(
        const RenderContext& ctx,
        const NodeRegistry& registry);

private:
    Shader* particleShader{ nullptr };
};

