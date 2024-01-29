#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"


namespace script
{
    class AudioStop final : public NodeCommand
    {
    public:
        AudioStop(
            ki::node_id nodeId,
            int index) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        int m_index;
    };
}
