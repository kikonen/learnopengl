#pragma once

#include <vector>
#include <memory>

#include "Renderer.h"

namespace render {
    class FrameBuffer;
    class NodeDraw;
}

class LayerRenderer final : public Renderer
{
public:
    static const int ATT_ALBEDO_INDEX = 0;
    static const int ATT_DEPTH_INDEX = 1;

public:
    LayerRenderer(
        std::string_view name,
        bool useFrameStep)
        : Renderer(name, useFrameStep) {}

    virtual ~LayerRenderer() override;

    void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(const UpdateViewContext& ctx);

    void render(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

private:
    void fillHighlightMask(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

    void renderHighlight(
        const RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

public:
    std::unique_ptr<render::FrameBuffer> m_buffer{ nullptr };

    std::unique_ptr<render::NodeDraw> m_nodeDraw;

private:
    Program* m_selectionProgram{ nullptr };

    int m_width{ -1 };
    int m_height{ -1 };

    int m_taggedCount{ 0 };
    int m_selectedCount{ 0 };
};
