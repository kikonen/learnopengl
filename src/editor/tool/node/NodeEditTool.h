#pragma once

#include "editor/tool/Tool.h"

#include "NodeEditToolState.h"

namespace editor
{
    class NodeEditTool : public Tool
    {
    public:
        NodeEditTool(EditorFrame& editor);
        ~NodeEditTool() override;

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
        void onSelectNode(
            const RenderContext& ctx,
            pool::NodeHandle nodeHandle);

        void onDeleteNode(
            const RenderContext& ctx,
            pool::NodeHandle nodeHandle);

    private:
        NodeEditToolState m_state;
    };
}
