#pragma once

#include "Command.h"

#include "pool/NodeHandle.h"

namespace script
{
    class NodeCommand : public Command
    {
    public:
        NodeCommand(
            pool::NodeHandle handle,
            float duration,
            bool relative) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "node_command";
        }

        virtual bool isNode() noexcept override final { return true; };

        virtual void bind(const UpdateContext& ctx) noexcept;

    protected:
        inline Node* getNode() noexcept
        {
            auto* node = m_handle.toNode();
            if (!node) {
                m_finished = true;
            }
            return node;
        }

    public:
        const pool::NodeHandle m_handle;
        const bool m_relative;
    };
}
