#pragma once

#include "Tool.h"

#include "NodeEditState.h"

namespace editor
{
    class NodeEditTool : public Tool
    {
    public:
        NodeEditTool(EditorFrame& editor);
        ~NodeEditTool() override;

        void draw(
            const RenderContext& ctx,
            render::DebugContext& dbg) override;

        void processInputs(
            const RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState) override;

    protected:
        void handleSelectNode(
            const RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState);

        void renderNodeEdit(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderNodeSelector(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderNodeProperties(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderTypeProperties(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderRigProperties(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderNodeDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderAnimationDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

    private:
        NodeEditState m_state;
    };
}
