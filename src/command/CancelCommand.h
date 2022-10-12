#pragma once

#include "command/Command.h"


class CancelCommand final : public Command
{
public:
    CancelCommand(
        float initialDelay,
        float finishTime,
        int commandId);

    virtual void execute(
        const RenderContext& ctx) override;

private:
    const int m_commandId;
};
