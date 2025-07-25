#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

#include "audio/size.h"

namespace script
{
    class AudioPlay final : public NodeCommand
    {
    public:
        // @param sync wait until audio stopped before finishing this command
        AudioPlay(
            pool::NodeHandle handle,
            audio::source_id audioSid,
            bool sync) noexcept;

        virtual std::string getName() const noexcept override
        {
            return "audio_play";
        }

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        audio::source_id m_audioSid;
        bool m_sync;

        bool m_started{ false };
    };
}
