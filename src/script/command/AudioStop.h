#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

#include "audio/size.h"

namespace script
{
    class AudioStop final : public NodeCommand
    {
    public:
        AudioStop(
            pool::NodeHandle handle,
            audio::source_id id) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "audio_stop";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        audio::source_id m_id;
    };
}
