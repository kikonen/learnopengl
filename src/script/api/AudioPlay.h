#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"


namespace script
{
    class AudioPlay final : public NodeCommand
    {
    public:
        AudioPlay(
            script::command_id afterCommandId,
            ki::node_id nodeId,
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
}
