#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"


namespace script
{
    class AudioPlay final : public NodeCommand
    {
    public:
        AudioPlay(
            ki::node_id nodeId,
            int index,
            bool sync) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        int m_index;
        bool m_sync;

        bool m_started{ false };
    };
}
