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
        sol::function& fn);

    virtual void bind(
        const RenderContext& ctx,
        Node* node) override;

    virtual void execute(
        const RenderContext& ctx) override;

private:
    sol::function& m_fn;
};
