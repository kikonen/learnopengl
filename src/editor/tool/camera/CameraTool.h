#pragma once

#include "editor/tool/Tool.h"

#include "CameraToolState.h"

namespace editor
{
    class CameraTool : public Tool
    {
    public:
        CameraTool(EditorFrame& editor);
        ~CameraTool() override;

        void processInputs(
            const InputContext& ctx) override;

    protected:
        void drawImpl(
            const gui::FrameContext& ctx) override;

        void renderCamera(
            const gui::FrameContext& ctx);

    private:
        CameraToolState m_state;
    };
}
