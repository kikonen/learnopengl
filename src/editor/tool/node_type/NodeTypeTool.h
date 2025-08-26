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

        void drawImpl(
            const RenderContext& ctx,
            Scene* scene,
            debug::DebugContext& dbg) override;

        void processInputs(
            const RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState) override;

    protected:
        void renderTypeEdit(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderTypeSelector(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderTypeProperties(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

    private:
        void onSelectType(
            const RenderContext& ctx,
            pool::TypeHandle TypeHandle);

        void onCreateNode(
            const RenderContext& ctx,
            pool::TypeHandle TypeHandle);

    private:
        NodeTypeToolState m_state;
    };
}
