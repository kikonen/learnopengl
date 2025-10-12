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

        void prepare(const PrepareContext& ctx) override;

        void drawImpl(
            const gui::FrameContext& ctx) override;

        void processInputs(
            const InputContext& ctx) override;

    protected:
        void renderBufferDebug(
            const gui::FrameContext& ctx);

        void renderSkyboxDebug(
            const gui::FrameContext& ctx);

    private:
        ViewportToolState m_state;
    };
}
