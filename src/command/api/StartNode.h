#pragma once

#include <sol/sol.hpp>

#include "NodeCommand.h"

class StartNode final : public NodeCommand
{
public:
    StartNode(
        int afterCommandId,
        int objectID,
        std::unique_ptr<sol::coroutine> coroutine,
        sol::variadic_args vargs) noexcept;

    virtual void bind(
        const RenderContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const RenderContext& ctx) noexcept override;

private:
    std::unique_ptr<sol::coroutine> m_coroutine;
    sol::variadic_args m_vargs;
};
