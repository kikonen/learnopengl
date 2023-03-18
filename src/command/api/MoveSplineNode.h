#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"


class MoveSplineNode final : public NodeCommand
{
public:
    MoveSplineNode(
        int afterCommandId,
        int objectID,
        float duration,
        bool relative,
        const glm::vec3& controlPoint,
        const glm::vec3& position) noexcept;

    virtual void bind(
        const UpdateContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const glm::vec3 m_controlPoint;

    const glm::vec3 m_position;
    glm::vec3 m_end{ 0.f };
    glm::vec3 m_previous{ 0.f };
};
