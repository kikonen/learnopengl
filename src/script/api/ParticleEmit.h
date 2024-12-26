#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

#include "audio/size.h"

namespace script
{
    class ParticleEmit final : public NodeCommand
    {
    public:
        ParticleEmit(
            pool::NodeHandle handle,
            float count,
            bool sync) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "particle_emit";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        float m_count;
        bool m_sync;

        bool m_started{ false };
    };
}
