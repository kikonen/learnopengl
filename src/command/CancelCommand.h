#pragma once

#include "command/Command.h"


class CancelCommand final : public Command
{
public:
    CancelCommand(
        int afterCommandId,
        float finishTime,
        int commandId) noexcept;

    virtual void execute(
        const RenderContext& ctx) noexcept override;

private:
    const int m_commandId;
};
