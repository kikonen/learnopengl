#pragma once

#include <glm/glm.hpp>

#include "command/Command.h"


class MoveNode : public Command
{
public:
    MoveNode(
        int objectID,
        float secs,
        const glm::vec3& pos);

    virtual bool execute(
        const RenderContext& ctx,
        Node& node) override;
};
