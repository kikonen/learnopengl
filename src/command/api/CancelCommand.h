#pragma once

#include "Command.h"


class CancelCommand final : public Command
{
public:
    CancelCommand(
        ki::command_id afterCommandId,
        float duration,
        ki::command_id commandId) noexcept;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const ki::command_id m_commandId;
};
