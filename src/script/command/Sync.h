#pragma once

#include "Command.h"

namespace script
{
    // TODO KI rename to "Barrier"
    class Sync final : public Command
    {
    public:
        Sync(
            float duration,
            const std::vector<script::command_id>& commandIds) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "sync";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        std::vector<script::command_id> m_commandIds;
        bool m_syncedAll{ false };
    };
}
