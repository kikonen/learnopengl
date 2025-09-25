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

        void drawMenuImpl(
            const render::RenderContext& ctx,
            Scene* scene,
            debug::DebugContext& dbg) override;

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
        void renderOptions(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

    private:
        OptionsToolState m_state;
    };
}
