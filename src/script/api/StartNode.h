#pragma once

#include "NodeCommand.h"

namespace script
{
    class Coroutine;

    class StartNode final : public NodeCommand
    {
    public:
        StartNode(
            script::command_id afterCommandId,
            ki::node_id nodeId,
            Coroutine* coroutine) noexcept;

        virtual void bind(
            const UpdateContext& ctx,
            Node* node) noexcept override;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        Coroutine* m_coroutine;
    };
}
