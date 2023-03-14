#pragma once

#include "Command.h"


class Wait final : public Command
{
public:
    Wait(
        int afterCommandId,
        float finishTime) noexcept;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
};
