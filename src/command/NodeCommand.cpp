
#include "command/NodeCommand.h"

NodeCommand::NodeCommand(
    int afterCommandId,
    int objectID,
    float initialDelay,
    float finishTime)
    : Command(afterCommandId, initialDelay, finishTime),
    m_objectID(objectID)
{
}

void NodeCommand::bind(const RenderContext& ctx, Node* node)
{
    Command::bind(ctx);
    m_node = node;
}
