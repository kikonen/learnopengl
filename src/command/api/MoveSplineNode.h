#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"


class MoveSplineNode final : public NodeCommand
{
public:
    MoveSplineNode(
        int afterCommandId,
        int objectID,
        float finishTime,
        bool relative,
        const glm::vec3& controlPoint,
        const glm::vec3& position) noexcept;

    virtual void bind(
        const RenderContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const RenderContext& ctx) noexcept override;

private:
    const glm::vec3 m_controlPoint;
    const glm::vec3 m_end;
    glm::vec3 m_begin;
};
