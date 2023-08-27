#pragma once

#include <vector>

#include "Renderer.h"

class FrameBuffer;

class NodeRenderer final : public Renderer
{
public:
    static const int ATT_ALBEDO_INDEX = 0;
    static const int ATT_DEPTH_INDEX = 1;

public:
    NodeRenderer(bool useFrameStep) : Renderer(useFrameStep) {}

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
    //Program* m_selectionProgramPointSprite{ nullptr };

    int m_taggedCount{ 0 };
    int m_selectedCount{ 0 };
};
