#pragma once

#include <vector>

#include "Renderer.h"

#include "asset/Uniform.h"

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
    Program* m_selectionProgramSprite{ nullptr };

    uniform::Int u_stencilMode{ "u_stencilMode", UNIFORM_STENCIL_MODE };
    uniform::Int u_stencilModeSprite{ "u_stencilMode", UNIFORM_STENCIL_MODE };

    int m_taggedCount{ 0 };
    int m_selectedCount{ 0 };
};
