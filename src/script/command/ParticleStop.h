#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

#include "audio/size.h"

namespace script
{
    class ParticleStop final : public NodeCommand
    {
    public:
        ParticleStop(
            pool::NodeHandle handle) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "particle_stop";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        bool m_started{ false };
    };
}
