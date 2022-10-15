#pragma once

#include <glm/glm.hpp>

#include "command/NodeCommand.h"


class MoveSplineNode final : public NodeCommand
{
public:
    MoveSplineNode(
        int afterCommandId,
        int objectID,
        float initialDelay,
        float finishTime,
        bool relative,
        const glm::vec3& controlPoint,
        const glm::vec3& position);

    virtual void bind(
        const RenderContext& ctx,
        Node* node) override;

    virtual void execute(
        const RenderContext& ctx) override;

private:
    const glm::vec3 m_controlPoint;
    const glm::vec3 m_end;
    glm::vec3 m_begin;
};
