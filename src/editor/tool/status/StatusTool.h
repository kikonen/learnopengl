#pragma once

#include "editor/tool/Tool.h"

#include "StatusToolState.h"

namespace editor
{
    class StatusTool : public Tool
    {
    public:
        StatusTool(EditorFrame& editor);
        ~StatusTool() override;

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
        void renderStatus(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

    private:
        StatusToolState m_state;
    };
}
