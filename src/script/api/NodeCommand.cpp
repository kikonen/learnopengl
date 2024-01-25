#include "NodeCommand.h"

#include "model/Node.h"

namespace script
{
    NodeCommand::NodeCommand(
        script::command_id afterCommandId,
        ki::node_id nodeId,
        float duration,
        bool relative) noexcept
        : Command(afterCommandId, duration),
        m_nodeId(nodeId),
        m_relative(relative)
    {
    }

    void NodeCommand::bind(const UpdateContext& ctx, Node* node) noexcept
    {
        Command::bind(ctx);
        m_nodeHandle = node->toHandle();
    }
}
