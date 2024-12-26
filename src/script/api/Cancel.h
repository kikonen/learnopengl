#pragma once

#include "Command.h"


namespace script
{
    class Cancel final : public Command
    {
    public:
        Cancel(
            float duration,
            script::command_id commandId) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "stop";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        script::command_id m_commandId;
    };
}
