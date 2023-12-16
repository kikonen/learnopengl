#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"


class AudioPause final : public NodeCommand
{
public:
    AudioPause(
        ki::command_id afterCommandId,
        ki::object_id nodeId,
        int index) noexcept;

    virtual void bind(
        const UpdateContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const int m_index;
};
