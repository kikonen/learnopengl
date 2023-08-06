#pragma once

#include <vector>

#include "Renderer.h"

class FrameBuffer;

class NodeRenderer final : public Renderer
{
public:
    NodeRenderer() {}

    void prepare(
        const Assets& assets,
        Registry* registry) override;

    void render(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer);

private:
    void renderStencil(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer);

    void renderHighlight(
        const RenderContext& ctx,
        FrameBuffer* targetBuffer);

private:
    Program* m_selectionProgram{ nullptr };
    Program* m_selectionProgramPointSprite{ nullptr };

    int m_taggedCount{ 0 };
    int m_selectedCount{ 0 };
};
