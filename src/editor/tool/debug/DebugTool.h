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
            const render::RenderContext& ctx,
            Scene* scene,
            debug::DebugContext& dbg) override;

        void processInputs(
            const render::RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState) override;

    protected:
        void renderPhysicsDebug(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderEffectDebug(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderLayersDebug(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

    private:
        DebugToolState m_state;
    };
}
