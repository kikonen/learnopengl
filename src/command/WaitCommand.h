#pragma once

#include "command/Command.h"


class WaitCommand final : public Command
{
public:
    WaitCommand(
        int afterCommandId,
        float finishTime) noexcept;

    virtual void execute(
        const RenderContext& ctx) noexcept override;

private:
};
