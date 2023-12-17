#pragma once

#include "Command.h"


namespace script
{
    class CancelCommand final : public Command
    {
    public:
        CancelCommand(
            script::command_id afterCommandId,
            float duration,
            script::command_id commandId) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const script::command_id m_commandId;
    };
}
