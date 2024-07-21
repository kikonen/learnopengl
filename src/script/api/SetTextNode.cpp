#include "SetTextNode.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "generator/TextGenerator.h"

namespace script
{
    SetTextNode::SetTextNode(
        pool::NodeHandle handle,
        float duration,
        std::string text) noexcept
        : NodeCommand(handle, duration, false),
        m_text(text)
    {}

    void SetTextNode::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;
        m_finished = m_elapsedTime >= m_duration;

        if (m_finished) {
            auto* generator = getNode()->getGenerator<TextGenerator>();
            if (generator) {
                generator->setText(m_text);
            }
        }
    }
}
