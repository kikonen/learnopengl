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
        void renderTypeEdit(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderTypeSelector(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderTypeProperties(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

    private:
        void onSelectType(
            const render::RenderContext& ctx,
            pool::TypeHandle TypeHandle);

        void onCreateNode(
            const render::RenderContext& ctx,
            pool::TypeHandle TypeHandle);

    private:
        NodeTypeToolState m_state;
    };
}
