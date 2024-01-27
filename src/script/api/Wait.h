#pragma once

#include "Command.h"

namespace script
{
    class Wait final : public Command
    {
    public:
        Wait(
            float duration) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
    };
}
