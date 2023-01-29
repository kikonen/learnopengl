#pragma once

#include "Command.h"


class Wait final : public Command
{
public:
    Wait(
        int afterCommandId,
        float finishTime) noexcept;

    virtual void execute(
        const RenderContext& ctx) noexcept override;

private:
};
