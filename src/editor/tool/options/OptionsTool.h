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

    void processInputs(
        const InputContext& ctx) override;

    protected:
        void drawMenuImpl(const gui::FrameContext& ctx) override;
        void drawImpl(const gui::FrameContext& ctx) override;

        void renderOptions(const gui::FrameContext& ctx);

    private:
        OptionsToolState m_state;
    };
}
