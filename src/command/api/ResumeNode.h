#pragma once

#include <sol/sol.hpp>

#include "NodeCommand.h"

class Coroutine;

class ResumeNode final : public NodeCommand
{
public:
    ResumeNode(
        ki::command_id afterCommandId,
        ki::object_id nodeId,
        Coroutine* coroutine) noexcept;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    Coroutine* m_coroutine;
};
