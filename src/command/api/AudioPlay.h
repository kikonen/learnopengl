#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"


class AudioPlay final : public NodeCommand
{
public:
    AudioPlay(
        ki::command_id afterCommandId,
        ki::object_id nodeId,
        int index,
        bool sync) noexcept;

    virtual void bind(
        const UpdateContext& ctx,
        Node* node) noexcept override;

    virtual void execute(
        const UpdateContext& ctx) noexcept override;

private:
    const int m_index;
    const bool m_sync;

    bool m_started{ false };
};
