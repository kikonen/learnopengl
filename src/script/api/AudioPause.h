#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

namespace script
{
    class AudioPause final : public NodeCommand
    {
    public:
        AudioPause(
            pool::NodeHandle handle,
            int index) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        int m_index;
    };
}
