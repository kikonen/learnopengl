#include "Command.h"

#include <mutex>

#include "engine/UpdateContext.h"

namespace {
    script::command_id idBase = 0;

    std::mutex id_lock{};

    script::command_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

namespace script
{
    Command::Command(
        script::command_id afterCommandId,
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
}
