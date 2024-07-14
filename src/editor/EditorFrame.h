#pragma once

#include "gui/Frame.h"

namespace render {
    struct DebugContext;
}

class EditorFrame : public Frame
{
public:
    EditorFrame(Window& window);

    void prepare(const PrepareContext& ctx) override;
    void draw(const RenderContext& ctx) override;

private:
    void trackImGuiState(
        render::DebugContext& debugContext);

    void renderStatus(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderCameraDebug(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderNodeSelector(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderNodeProperties(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderNodeDebug(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderAnimationDebug(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderBufferDebug(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderMiscDebug(
        const RenderContext& ctx,
        render::DebugContext& debugContext);
};

