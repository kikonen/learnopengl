#pragma once

#include "Command.h"


namespace script
{
    class CancelCommand final : public Command
    {
    public:
        CancelCommand(
            float duration,
            script::command_id commandId) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        script::command_id m_commandId;
    };
}
