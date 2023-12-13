#pragma once

#include "Command.h"

// TODO KI rename to "Barrier"
class Sync final : public Command
{
public:
    Sync(
        ki::command_id afterCommandId,
        float duration,
        const std::vector<ki::command_id>& commandIds) noexcept;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const std::vector<ki::command_id> m_commandIds;
    bool m_syncedAll = false;
};
