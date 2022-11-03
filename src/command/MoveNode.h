#pragma once

#include <glm/glm.hpp>

#include "command/NodeCommand.h"


class MoveNode final : public NodeCommand
{
public:
    MoveNode(
        int afterCommandId,
        int objectID,
        float finishTime,
        bool relative,
        const glm::vec3& position) noexcept;

    virtual void bind(
        const RenderContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const RenderContext& ctx) noexcept override;

private:
    const glm::vec3 m_end;
    glm::vec3 m_begin;
};
