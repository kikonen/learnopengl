#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

namespace script
{
    class AnimationPlay final : public NodeCommand
    {
    public:
        AnimationPlay(
            ki::node_id nodeId,
            std::string clip,
            bool repeat) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        std::string m_clip;
        bool m_repeat;
    };
}
