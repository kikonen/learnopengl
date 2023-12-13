#pragma once

#include "Command.h"


class Wait final : public Command
{
public:
    Wait(
        ki::command_id afterCommandId,
        float duration) noexcept;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
};
