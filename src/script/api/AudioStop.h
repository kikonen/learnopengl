#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"


namespace script
{
    class AudioStop final : public NodeCommand
    {
    public:
        AudioStop(
            script::command_id afterCommandId,
            ki::node_id nodeId,
            int index) noexcept;

        virtual void bind(
            const UpdateContext& ctx,
            Node* node) noexcept override;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const int m_index;
    };
}
