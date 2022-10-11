#pragma once

#include <glm/glm.hpp>

#include "command/Command.h"


class RotateNode final : public Command
{
public:
    RotateNode(
        int objectID,
        float finishTime,
        const glm::vec3& rotation);

    virtual void bind(
        const RenderContext& ctx,
        Node* node) override;

    virtual void execute(
        const RenderContext& ctx) override;

private:
    const glm::vec3 m_end;
    glm::vec3 m_begin;
};
