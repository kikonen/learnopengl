#include "SetVisibleNode.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

namespace script
{
    SetVisibleNode::SetVisibleNode(
        pool::NodeHandle handle,
        bool visible) noexcept
        : NodeCommand(handle, 0, false),
        m_visible{ visible }
    {
    }

    void SetVisibleNode::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* node = getNode();
        if (!node) return;

        node->m_visible = m_visible;
        m_finished = true;
    }
}
