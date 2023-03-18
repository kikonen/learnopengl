#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

class ScaleNode final : public NodeCommand
{
public:
    ScaleNode(
        int afterCommandId,
        int objectID,
        float duration,
        bool relative,
        const glm::vec3& scale) noexcept;

    virtual void bind(
        const UpdateContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const glm::vec3 m_scale;
    glm::vec3 m_end{ 0.f };
    glm::vec3 m_previous{ 0.f };
};
