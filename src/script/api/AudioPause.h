#pragma once

#include <glm/glm.hpp>

#include "NodeCommand.h"

#include "audio/size.h"

namespace script
{
    class AudioPause final : public NodeCommand
    {
    public:
        AudioPause(
            pool::NodeHandle handle,
            audio::source_id id) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        audio::source_id m_id;
    };
}
