#include "NodeCommand.h"

#include "model/Node.h"

namespace script
{
    NodeCommand::NodeCommand(
        ki::node_id nodeId,
        float duration,
        bool relative) noexcept
        : Command(duration),
        m_nodeId(nodeId),
        m_relative(relative)
    {
    }

    void NodeCommand::bind(const UpdateContext& ctx) noexcept
    {
        Command::bind(ctx);
        m_nodeHandle = pool::NodeHandle::toHandle(m_nodeId);
        m_finished = !m_nodeHandle.toNode();
    }
}
