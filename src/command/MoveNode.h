#pragma once

#include <glm/glm.hpp>

#include "command/NodeCommand.h"


class MoveNode final : public NodeCommand
{
public:
    MoveNode(
        int objectID,
        float initialDelay,
        float finishTime,
        const glm::vec3& position);

    virtual void bind(
        const RenderContext& ctx,
        Node* node) override;

    virtual void execute(
        const RenderContext& ctx) override;

private:
    const glm::vec3 m_end;
    glm::vec3 m_begin;
};
