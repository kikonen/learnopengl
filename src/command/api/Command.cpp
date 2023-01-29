#include "Command.h"

#include <mutex>

#include "scene/RenderContext.h"

namespace {
    int idBase = 0;

    std::mutex id_lock;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}


Command::Command(
    int afterCommandId,
    float finishTime) noexcept
    : m_id(nextID()),
    m_afterCommandId(afterCommandId),
    m_finishTime(finishTime)
{
}

void Command::bind(const RenderContext& ctx) noexcept
{
    m_elapsedTime = 0.f;
}
