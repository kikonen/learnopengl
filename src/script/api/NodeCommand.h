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

        virtual bool isNode() noexcept override final { return true; };

        virtual void bind(const UpdateContext& ctx) noexcept;

    protected:
        inline Node* getNode() const noexcept {
            return m_handle.toNode();
        }

    public:
        const pool::NodeHandle m_handle;
        const bool m_relative;
    };
}
