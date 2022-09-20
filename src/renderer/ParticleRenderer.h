#pragma once

#include "Renderer.h"

class ParticleRenderer final : public Renderer
{
public:
    ParticleRenderer();

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders) override;
    virtual void update(const RenderContext& ctx, const NodeRegistry& registry);
    virtual void bind(const RenderContext& ctx);
    virtual void render(const RenderContext& ctx, const NodeRegistry& registry);

private:
    std::shared_ptr<Shader> particleShader;
};

