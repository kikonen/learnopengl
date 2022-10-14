
#include "command/NodeCommand.h"

NodeCommand::NodeCommand(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float finishTime,
    bool relative)
    : Command(afterCommandId, initialDelay, finishTime),
    m_objectID(objectID),
    m_relative(relative)
{
}

void NodeCommand::bind(const RenderContext& ctx, Node* node)
{
    Command::bind(ctx);
    m_node = node;
}
