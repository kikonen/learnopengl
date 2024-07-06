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

    void renderNodeSelector(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderNodeEdit(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderNodeDebug(
        const RenderContext& ctx,
        render::DebugContext& debugContext);

    void renderBoneDebug(
        const RenderContext& ctx,
        render::DebugContext& debugContext);
};

