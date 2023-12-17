#pragma once

#include "Command.h"

namespace script
{
    // TODO KI rename to "Barrier"
    class Sync final : public Command
    {
    public:
        Sync(
            script::command_id afterCommandId,
            float duration,
            const std::vector<script::command_id>& commandIds) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const std::vector<script::command_id> m_commandIds;
        bool m_syncedAll = false;
    };
}
