#include "Command.h"

#include "engine/UpdateContext.h"

namespace script
{
    Command::Command(
        float duration) noexcept
        : m_duration(duration)
    {
    }

    void Command::bind(const UpdateContext& ctx) noexcept
    {
        m_elapsedTime = 0.f;
    }
}
