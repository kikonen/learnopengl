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

        void draw(
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
        void renderCameraDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

    private:
        CameraToolState m_state;
    };
}
