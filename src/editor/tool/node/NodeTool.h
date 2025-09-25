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
        void handleSelectNode(
            const render::RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState);

        void renderNode(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderNodeSelector(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderNodeProperties(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderTypeProperties(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderRigProperties(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderNodeDebug(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

        void renderAnimationDebug(
            const render::RenderContext& ctx,
            debug::DebugContext& dbg);

    private:
        void onSelectNode(
            const render::RenderContext& ctx,
            pool::NodeHandle nodeHandle);

        void onDeleteNode(
            const render::RenderContext& ctx,
            pool::NodeHandle nodeHandle);

        void onCloneNode(
            const render::RenderContext& ctx,
            pool::NodeHandle nodeHandle);

        void updateSocket(
            model::Node* node,
            mesh::Mesh* mesh,
            animation::RigContainer* rig,
            animation::RigSocket* socket);

    private:
        NodeToolState m_state;
    };
}
