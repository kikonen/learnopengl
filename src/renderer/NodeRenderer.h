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

    void render(
        const RenderContext& ctx);

private:
    void renderStencil(const RenderContext& ctx);
    void renderSelection(const RenderContext& ctx);

    void drawNodes(
        const RenderContext& ctx,
        bool selection);

    void drawBlended(
        const RenderContext& ctx);

    void drawStencil(const RenderContext& ctx);

private:
    Program* m_selectionProgram{ nullptr };
    Program* m_selectionProgramSprite{ nullptr };

    int m_taggedCount{ 0 };
    int m_selectedCount{ 0 };
};
