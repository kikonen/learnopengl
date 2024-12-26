#pragma once

#include "NodeCommand.h"

namespace script
{
    class Coroutine;

    class StartNode final : public NodeCommand
    {
    public:
        StartNode(
            pool::NodeHandle handle,
            Coroutine* coroutine) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "start_node";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        Coroutine* m_coroutine;
    };
}
