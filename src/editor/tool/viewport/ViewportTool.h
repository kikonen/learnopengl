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
        void renderBufferDebug(
            const RenderContext& ctx,
            Scene* scene,
            render::DebugContext& dbg);

    private:
        ViewportToolState m_state;
    };
}
