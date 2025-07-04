#include "SelectNode.h"

#include "engine/UpdateContext.h"

#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/SelectionRegistry.h"


namespace script
{
    SelectNode::SelectNode(
        pool::NodeHandle handle,
        bool select,
        bool append) noexcept
        : NodeCommand(handle, 0, false),
        m_select{ select },
        m_append{ append }
    {
    }

    void SelectNode::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished)
        {
            auto* node = getNode();
            if (!node) return;

            auto* dispatcherView = ctx.m_registry->m_dispatcherView;
            {
                event::Event evt{ event::Type::node_select };
                evt.body.select = {
                    .target = m_handle.toId(),
                    .select = m_select,
                    .append = m_append
                };
                assert(evt.body.node.target > 1);
                dispatcherView->send(evt);
            }
        }
    }
}
