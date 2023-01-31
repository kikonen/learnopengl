#pragma once

#include <vector>

#include "Renderer.h"

class NodeRenderer final : public Renderer
{
public:
    NodeRenderer();

    void prepare(
        const Assets& assets,
        Registry* registry) override;

    void update(const RenderContext& ctx) override;

    void render(
        const RenderContext& ctx);

private:
    void renderSelectionStencil(const RenderContext& ctx);
    void renderSelection(const RenderContext& ctx);

    void drawNodes(
        const RenderContext& ctx,
        bool selection);

    void drawBlended(
        const RenderContext& ctx);

    void drawSelectionStencil(const RenderContext& ctx);

private:
    Shader* m_selectionShader{ nullptr };
    Shader* m_selectionShaderSprite{ nullptr };

    int m_taggedCount{ 0 };
    int m_selectedCount{ 0 };
};
