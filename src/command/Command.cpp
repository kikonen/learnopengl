
#include "command/Command.h"


Command::Command(
    int objectID,
    float initialDelay,
    float finishTime)
    : m_objectID(objectID),
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

void Command::bind(const RenderContext& ctx, Node* node)
{
    m_node = node;
    m_elapsedTime = 0.f;
}
