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
            unsigned int id) noexcept;

        virtual void execute(
            const UpdateContext& ctx) noexcept override;

    private:
        audio::source_id m_id;
    };
}
