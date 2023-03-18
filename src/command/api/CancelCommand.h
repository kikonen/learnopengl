#pragma once

#include "Command.h"


class CancelCommand final : public Command
{
public:
    CancelCommand(
        int afterCommandId,
        float duration,
        int commandId) noexcept;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const int m_commandId;
};
