#pragma once

#include <vector>

#include "Renderer.h"
#include "SkyboxRenderer.h"

class NodeRenderer final : public Renderer
{
public:
    NodeRenderer();

    void prepare(const Assets& assets, ShaderRegistry& shaders) override;
    void update(const RenderContext& ctx) override;

    void render(
        const RenderContext& ctx,
        SkyboxRenderer* skybox);

private:
    void renderSelectionStencil(const RenderContext& ctx);
    void renderSelection(const RenderContext& ctx);

//    void drawBlended(const RenderContext& ctx);

    void drawNodes(
        const RenderContext& ctx,
        SkyboxRenderer* skybox,
        bool selection);

    void drawBlended(
        const RenderContext& ctx);

    void drawSelectionStencil(const RenderContext& ctx);

private:
    Shader* m_selectionShader{ nullptr };
    Shader* m_selectionShaderAlpha{ nullptr };
    Shader* m_selectionShaderSprite{ nullptr };

    int m_selectedCount{ 0 };
};
