#pragma once

#include "gui/Frame.h"

#include "EditorState.h"

namespace render {
    struct DebugContext;
}

namespace editor {
    class EditorFrame : public Frame
    {
    public:
        EditorFrame(Window& window);

        void prepare(const PrepareContext& ctx) override;
        void draw(const RenderContext& ctx) override;

        EditorState& getState()
        {
            return m_state;
        }

    private:
        void trackImGuiState(
            render::DebugContext& dbg);

        void renderMenuBar(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderStatus(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderCameraDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderNodeEdit(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderNodeSelector(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderNodeProperties(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderTypeProperties(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderRigProperties(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderNodeDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderAnimationDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderBufferDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderPhysicsDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderEffectDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderMiscDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

    private:
        EditorState m_state;
    };
}
