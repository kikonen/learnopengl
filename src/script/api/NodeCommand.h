#pragma once

#include "Command.h"

class Node;

namespace script
{
    class NodeCommand : public Command
    {
    public:
        NodeCommand(
            script::command_id afterCommandId,
            ki::object_id nodeId,
            float duration,
            bool relative) noexcept;

        virtual bool isNode() noexcept override final { return true; };

        virtual void bind(const UpdateContext& ctx, Node* node) noexcept;

    public:
        const ki::object_id m_nodeId;
        const bool m_relative;

        Node* m_node{ nullptr };
    };
}
