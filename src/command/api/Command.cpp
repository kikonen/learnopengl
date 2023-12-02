#include "Command.h"

#include <mutex>

#include "engine/UpdateContext.h"

namespace {
    ki::command_id idBase = 0;

    std::mutex id_lock{};

    ki::command_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}


Command::Command(
    ki::command_id afterCommandId,
    float duration) noexcept
    : m_id(nextID()),
    m_afterCommandId(afterCommandId),
    m_duration(duration)
{
}

void Command::bind(const UpdateContext& ctx) noexcept
{
    m_elapsedTime = 0.f;
}
