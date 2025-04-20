#pragma once

#include "Command.h"


namespace script
{
    class Cancel final : public Command
    {
    public:
        Cancel(
            float duration,
            std::vector<script::command_id> commandIds) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "stop";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        std::vector<script::command_id> m_commandIds;
    };
}
