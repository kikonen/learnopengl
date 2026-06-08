#include "SetVisibleNode.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "registry/Registry.h"

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

        // NOTE KI notify RT to update per-drawable hidden flag (WT => RT, like node_added)
        {
            event::Event evt{ event::Type::node_visible };
            evt.body.node.target = node->toHandle();
            ctx.getRegistry()->m_dispatcherView->send(evt);
        }

        m_finished = true;
    }
}
