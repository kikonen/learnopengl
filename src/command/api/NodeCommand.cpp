#include "NodeCommand.h"

NodeCommand::NodeCommand(
    int afterCommandId,
    int objectID,
    float duration,
    bool relative) noexcept
    : Command(afterCommandId, duration),
    m_objectID(objectID),
    m_relative(relative)
{
}

void NodeCommand::bind(const UpdateContext& ctx, Node* node) noexcept
{
    Command::bind(ctx);
    m_node = node;
}
