#pragma once

#include "Renderer.h"

class ViewportRenderer final : public Renderer
{
public:
    ViewportRenderer();

    void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    void update(const RenderContext& ctx, const NodeRegistry& registry) override;
    void bind(const RenderContext& ctx) override;
    void render(const RenderContext& ctx, const NodeRegistry& registry) override;
};

