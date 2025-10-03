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
            const gui::FrameContext& ctx) override;

        void processInputs(
            const InputContext& ctx) override;

    protected:
        void renderPhysicsDebug(
            const gui::FrameContext& ctx);

        void renderEffectDebug(
            const gui::FrameContext& ctx);

        void renderLayersDebug(
            const gui::FrameContext& ctx);

    private:
        DebugToolState m_state;
    };
}
