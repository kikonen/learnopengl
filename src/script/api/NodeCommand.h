#pragma once

#include "Command.h"

#include "pool/NodeHandle.h"

class Node;

namespace script
{
    class NodeCommand : public Command
    {
    public:
        NodeCommand(
            script::command_id afterCommandId,
            ki::node_id nodeId,
            float duration,
            bool relative) noexcept;

        virtual bool isNode() noexcept override final { return true; };

        virtual void bind(const UpdateContext& ctx, Node* node) noexcept;

    protected:
        inline Node* getNode() const noexcept {
            return m_nodeHandle.toNode();
        }

    public:
        const ki::node_id m_nodeId;
        const bool m_relative;

        pool::NodeHandle m_nodeHandle{};
    };
}
