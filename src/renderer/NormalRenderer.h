#pragma once

#include "Renderer.h"

class NormalRenderer final : public Renderer
{
public:
    NormalRenderer();

    virtual void prepare(const Assets& assets, ShaderRegistry& shaders) override;

    void render(
        const RenderContext& ctx);

private:
    void drawNodes(const RenderContext& ctx);

private:
    Shader* m_normalShader{ nullptr };
};

