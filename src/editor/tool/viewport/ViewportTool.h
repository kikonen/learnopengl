#pragma once

#include "editor/tool/Tool.h"

#include "ViewportToolState.h"

namespace editor
{
    class ViewportTool : public Tool
    {
    public:
        ViewportTool(EditorFrame& editor);
        ~ViewportTool() override;

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
        void renderBufferDebug(
            const render::RenderContext& ctx,
            Scene* scene,
            debug::DebugContext& dbg);

    private:
        ViewportToolState m_state;
    };
}
