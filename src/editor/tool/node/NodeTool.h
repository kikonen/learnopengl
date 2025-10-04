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
            const gui::FrameContext& ctx) override;

        void processInputs(
            const InputContext& ctx) override;

    protected:
        void handleSelectNode(
            const InputContext& ctx);

        void renderNode(
            const gui::FrameContext& ctx);

        void renderNodeSelector(
            const gui::FrameContext& ctx);

        void renderNodeProperties(
            const gui::FrameContext& ctx);

        void renderTypeProperties(
            const gui::FrameContext& ctx);

        void renderRigProperties(
            const gui::FrameContext& ctx);

        void renderNodeDebug(
            const gui::FrameContext& ctx);

        void renderAnimationDebug(
            const gui::FrameContext& ctx);

    private:
        void onSelectNode(
            const gui::FrameContext& ctx,
            pool::NodeHandle nodeHandle);

        void onDeleteNode(
            const gui::FrameContext& ctx,
            pool::NodeHandle nodeHandle);

        void onCloneNode(
            const gui::FrameContext& ctx,
            pool::NodeHandle nodeHandle);

        void updateSocket(
            model::Node* node,
            mesh::Mesh* mesh,
            animation::RigContainer* rig,
            animation::RigSocket* socket);

    private:
        NodeToolState m_state;

        event::Listen m_listen_node_select;
    };
}
