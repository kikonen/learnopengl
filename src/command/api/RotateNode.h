#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

class RotateNode final : public NodeCommand
{
public:
    RotateNode(
        int afterCommandId,
        int objectID,
        float finishTime,
        bool relative,
        const glm::vec3& rotation) noexcept;

    virtual void bind(
        const RenderContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const RenderContext& ctx) noexcept override;

private:
    const glm::vec3 m_end;
    glm::vec3 m_begin;
};
