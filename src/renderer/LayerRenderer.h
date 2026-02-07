#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

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
        bool useFrameStep,
        bool useHighlight)
        : Renderer(name, useFrameStep),
        m_useHighlight{ useHighlight }
    {}

    virtual ~LayerRenderer() override;

    void prepareRT(
        const PrepareContext& ctx) override;

    void updateRT(const UpdateViewContext& ctx);

    void render(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

private:
    void fillHighlightMask(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

    void renderHighlight(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

    void renderSelectionWireframe(
        const render::RenderContext& ctx,
        render::FrameBuffer* targetBuffer);

public:
    std::unique_ptr<render::FrameBuffer> m_buffer{ nullptr };

    std::unique_ptr<render::NodeDraw> m_nodeDraw;

    glm::u16vec2 m_aspectRatio{ 1, 1 };

private:
    Program* m_selectionProgram{ nullptr };

    const bool m_useHighlight;

    int m_width{ -1 };
    int m_height{ -1 };

    size_t m_taggedCount{ 0 };
    size_t m_selectedCount{ 0 };
};
