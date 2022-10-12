#include "command/Command.h"

#include <mutex>

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
    float initialDelay,
    float finishTime)
    : m_id(nextID()),
    m_initialDelay(initialDelay),
    m_finishTime(finishTime),
    m_ready(initialDelay <= 0.f)
{
}

void Command::wait(const RenderContext& ctx)
{
    m_elapsedTime += ctx.clock.elapsedSecs;
    m_ready = m_elapsedTime >= m_initialDelay;
}

void Command::bind(const RenderContext& ctx)
{
    m_elapsedTime = 0.f;
}
