#pragma once

#include "command/Command.h"


class WaitCommand final : public Command
{
public:
    WaitCommand(
        int afterCommandId,
        float finishTime);

    virtual void execute(
        const RenderContext& ctx) override;

private:
};
