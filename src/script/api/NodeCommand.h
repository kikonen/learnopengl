#pragma once

#include "Command.h"

#include "pool/NodeHandle.h"

namespace script
{
    class NodeCommand : public Command
    {
    public:
        NodeCommand(
            ki::node_id nodeId,
            float duration,
            bool relative) noexcept;

        virtual bool isNode() noexcept override final { return true; };

        virtual void bind(const UpdateContext& ctx) noexcept;

    protected:
        inline Node* getNode() const noexcept {
            return m_nodeHandle.toNode();
        }

    public:
        ki::node_id m_nodeId;
        bool m_relative;

        pool::NodeHandle m_nodeHandle{};
    };
}
