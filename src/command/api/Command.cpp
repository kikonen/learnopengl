#include "Command.h"

#include <mutex>

#include "engine/UpdateContext.h"

namespace {
    int idBase = 0;

    std::mutex id_lock{};

    int nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}


Command::Command(
    int afterCommandId,
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
