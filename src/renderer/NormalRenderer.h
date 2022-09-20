#pragma once

#include "Renderer.h"

class NormalRenderer final : public Renderer
{
public:
    NormalRenderer();

    void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    void bind(const RenderContext& ctx) override;
    void render(const RenderContext& ctx, const NodeRegistry& registry) override;

private:
    void drawNodes(const RenderContext& ctx, const NodeRegistry& registry);

private:
    std::shared_ptr<Shader> normalShader = nullptr;
};

