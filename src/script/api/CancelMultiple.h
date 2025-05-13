#pragma once

#include "Command.h"


namespace script
{
    class CancelMultiple final : public Command
    {
    public:
        CancelMultiple(
            float duration,
            std::vector<script::command_id> commandIds) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "cancel_multiple";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        std::vector<script::command_id> m_commandIds;
    };
}
