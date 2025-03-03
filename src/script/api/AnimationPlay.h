#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

namespace script
{
    class AnimationPlay final : public NodeCommand
    {
    public:
        AnimationPlay(
            pool::NodeHandle handle,
            ki::sid_t clipId,
            float speed,
            bool repeat) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "animation_play";
        }

        virtual void bind(const UpdateContext& ctx) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        const ki::sid_t m_clipId;
        const float m_speed;
        const bool m_repeat;
        int16_t m_clipIndex{ -1 };
    };
}
