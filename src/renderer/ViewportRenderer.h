#pragma once

#include "Renderer.h"

class ViewportRenderer final : public Renderer
{
public:
    ViewportRenderer();

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    virtual void update(const RenderContext& ctx) override;

    void render(
        const RenderContext& ctx);
};

