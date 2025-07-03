#pragma once

#include "editor/tool/Tool.h"

#include "OptionsToolState.h"

namespace editor
{
    class OptionsTool : public Tool
    {
    public:
        OptionsTool(EditorFrame& editor);
        ~OptionsTool() override;

        void drawMenu(
            const RenderContext& ctx,
            Scene* scene,
            render::DebugContext& dbg) override;

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
        void renderMiscDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

    private:
        OptionsToolState m_state;
    };
}
