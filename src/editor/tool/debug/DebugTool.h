#pragma once

#include "editor/tool/Tool.h"

#include "DebugToolState.h"

namespace editor
{
    class DebugTool : public Tool
    {
    public:
        DebugTool(EditorFrame& editor);
        ~DebugTool() override;

        void drawImpl(
            const RenderContext& ctx,
            Scene* scene,
            render::DebugContext& dbg) override;

        void processInputs(
            const RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState) override;

    protected:
        void renderPhysicsDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderEffectDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderLayersDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

    private:
        DebugToolState m_state;
    };
}
