#include "ResumeNode.h"

#include "scene/RenderContext.h"

#include "command/ScriptEngine.h"


ResumeNode::ResumeNode(
    int afterCommandId,
    int objectID,
    const std::string& callbackFn) noexcept
    : NodeCommand(afterCommandId, objectID, 0, false),
    m_callbackFn(callbackFn)
{
}

void ResumeNode::execute(
    const RenderContext& ctx) noexcept
{
    m_elapsedTime += ctx.m_clock.elapsedSecs;

    m_finished = m_elapsedTime >= m_finishTime;
    if (m_finished) {
        ctx.m_scriptEngine->invokeFunction(m_node, m_callbackFn);
    }
}
