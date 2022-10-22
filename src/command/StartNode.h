#pragma once

#include <sol/sol.hpp>

#include "command/NodeCommand.h"

class StartNode final : public NodeCommand
{
public:
    StartNode(
        int afterCommandId,
        int objectID,
        float initialDelay,
        std::unique_ptr<sol::coroutine> coroutine,
        sol::variadic_args vargs);

    virtual void bind(
        const RenderContext& ctx,
        Node* node) override;

    virtual void execute(
        const RenderContext& ctx) override;

private:
    std::unique_ptr<sol::coroutine> m_coroutine;
    sol::variadic_args m_vargs;
};
