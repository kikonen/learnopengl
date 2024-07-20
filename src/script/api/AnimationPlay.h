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
            std::string clipName,
            float speed,
            bool repeat) noexcept;

        virtual void bind(const UpdateContext& ctx) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const std::string m_clipName;
        const float m_speed;
        const bool m_repeat;
        int16_t m_clipIndex{ -1 };
    };
}
