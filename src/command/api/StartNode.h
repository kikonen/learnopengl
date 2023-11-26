#pragma once

#include "NodeCommand.h"

class Coroutine;

class StartNode final : public NodeCommand
{
public:
    StartNode(
        int afterCommandId,
        int objectID,
        Coroutine* coroutine) noexcept;

    virtual void bind(
        const UpdateContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    Coroutine* m_coroutine;
};
