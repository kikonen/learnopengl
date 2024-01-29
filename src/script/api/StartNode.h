#pragma once

#include "NodeCommand.h"

namespace script
{
    class Coroutine;

    class StartNode final : public NodeCommand
    {
    public:
        StartNode(
            ki::node_id nodeId,
            Coroutine* coroutine) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        Coroutine* m_coroutine;
    };
}
