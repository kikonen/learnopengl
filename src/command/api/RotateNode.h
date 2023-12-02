#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

class RotateNode final : public NodeCommand
{
public:
    RotateNode(
        ki::command_id afterCommandId,
        ki::object_id nodeId,
        float duration,
        bool relative,
        const glm::vec3& rotation) noexcept;

    virtual void bind(
        const UpdateContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const glm::vec3 m_rotation;
    glm::vec3 m_end{ 0.f };
    glm::vec3 m_previous{ 0.f };
};
