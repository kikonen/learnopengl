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
            const gui::FrameContext& ctx) override;

        void processInputs(
            const InputContext& ctx) override;

    protected:
        void renderStatus(
            const gui::FrameContext& ctx);

    private:
        StatusToolState m_state;
    };
}
