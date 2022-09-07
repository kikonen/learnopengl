#pragma once

#include "Renderer.h"

class NormalRenderer final : public Renderer
{
public:
    NormalRenderer(const Assets& assets);

    void prepare(ShaderRegistry& shaders) override;

    void bind(const RenderContext& ctx) override;
    void render(const RenderContext& ctx, NodeRegistry& registry) override;

private:
    void drawNodes(const RenderContext& ctx, NodeRegistry& registry);

private:
    std::shared_ptr<Shader> normalShader = nullptr;
};

