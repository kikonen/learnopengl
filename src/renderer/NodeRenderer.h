#pragma once

#include <vector>

#include "Renderer.h"
#include "SkyboxRenderer.h"

class NodeRenderer final : public Renderer
{
public:
    NodeRenderer();

    void prepare(const Assets& assets, ShaderRegistry& shaders) override;
    void update(const RenderContext& ctx, const NodeRegistry& registry) override;
    void bind(const RenderContext& ctx) override;

    void render(
        const RenderContext& ctx,
        const NodeRegistry& registry,
        SkyboxRenderer* skybox);

private:
    void renderSelectionStencil(const RenderContext& ctx, const NodeRegistry& registry);
    void renderSelection(const RenderContext& ctx, const NodeRegistry& registry);

//    void drawBlended(const RenderContext& ctx, const NodeRegistry& registry);

    void drawNodes(
        const RenderContext& ctx,
        const NodeRegistry& registry,
        SkyboxRenderer* skybox,
        bool selection);

    void drawBlended(
        const RenderContext& ctx,
        const NodeRegistry& registry);

    void drawSelectionStencil(const RenderContext& ctx, const NodeRegistry& registry);

private:
    Shader* m_selectionShader{ nullptr };
    Shader* m_selectionShaderAlpha{ nullptr };
    Shader* m_selectionShaderSprite{ nullptr };

    int m_selectedCount{ 0 };
};
