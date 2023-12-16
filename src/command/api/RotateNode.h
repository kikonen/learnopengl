#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "NodeCommand.h"

class RotateNode final : public NodeCommand
{
public:
    RotateNode(
        ki::command_id afterCommandId,
        ki::object_id nodeId,
        float duration,
        bool relative,
        const glm::vec3& axis,
        const float degrees) noexcept;

    virtual void bind(
        const UpdateContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const glm::vec3 m_axis;
    const float m_radians;

    glm::quat m_base{ 1.f, 0.f, 0.f, 0.f };
    glm::vec3 m_relativeAxis{ 0.f };

    glm::quat m_start{ 1.f, 0.f, 0.f, 0.f };
};
