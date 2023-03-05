#pragma once

#include "Command.h"

// TODO KI rename to "Barrier"
class Sync final : public Command
{
public:
    Sync(
        int afterCommandId,
        float finishTime,
        std::vector<int> commandIds) noexcept;

    virtual void execute(
        const RenderContext& ctx) noexcept override;

private:
    std::vector<int> m_commandIds;
    bool m_syncedAll = false;
};
