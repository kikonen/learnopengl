#pragma once

#include "editor/tool/Tool.h"

#include "NodeTypeToolState.h"

namespace editor
{
    class NodeTypeTool : public Tool
    {
    public:
        NodeTypeTool(EditorFrame& editor);
        ~NodeTypeTool() override;

        void clear() override;

        void drawImpl(
            const gui::FrameContext& ctx) override;

        void processInputs(
            const InputContext& ctx) override;

    protected:
        void renderTypeEdit(
            const gui::FrameContext& ctx);

        void renderTypeSelector(
            const gui::FrameContext& ctx);

        void renderTypeProperties(
            const gui::FrameContext& ctx);

    private:
        void onSelectType(
            const gui::FrameContext& ctx,
            pool::TypeHandle TypeHandle);

        void onCreateNode(
            const gui::FrameContext& ctx,
            pool::TypeHandle TypeHandle);

    private:
        NodeTypeToolState m_state;
    };
}
