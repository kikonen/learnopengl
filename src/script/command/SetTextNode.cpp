#include "SetTextNode.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "generator/TextGenerator.h"

namespace script
{
    SetTextNode::SetTextNode(
        pool::NodeHandle handle,
        std::string text) noexcept
        : NodeCommand(handle, 0, false),
        m_text(text)
    {}

    void SetTextNode::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* node = getNode();
        if (!node) return;

        auto* generator = node->getGenerator<TextGenerator>();
        if (generator) {
            generator->setText(m_text);
        }
        m_finished = true;
    }
}
