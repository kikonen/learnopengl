#pragma once

#include "editor/tool/Tool.h"

#include "NodeToolState.h"

namespace mesh {
    class Mesh;
}

namespace animation {
    struct RigContainer;
    struct RigSocket;
}

namespace editor
{
    struct NodeTree;

    class NodeTool : public Tool
    {
        friend struct NodeTree;

    public:
        NodeTool(EditorFrame& editor);
        ~NodeTool() override;

        void prepare(const PrepareContext& ctx) override;

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
        void handleSelectNode(
            const RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState);

        void renderNode(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderNodeSelector(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderNodeProperties(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderTypeProperties(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderRigProperties(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderNodeDebug(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderAnimationDebug(
            const RenderContext& ctx,
            debug::DebugContext& dbg);

    private:
        void onSelectNode(
            const RenderContext& ctx,
            pool::NodeHandle nodeHandle);

        void onDeleteNode(
            const RenderContext& ctx,
            pool::NodeHandle nodeHandle);

        void onCloneNode(
            const RenderContext& ctx,
            pool::NodeHandle nodeHandle);

        void updateSocket(
            Node* node,
            mesh::Mesh* mesh,
            animation::RigContainer* rig,
            animation::RigSocket* socket);

    private:
        NodeToolState m_state;
    };
}
