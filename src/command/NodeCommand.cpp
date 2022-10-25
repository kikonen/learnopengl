
#include "command/NodeCommand.h"

NodeCommand::NodeCommand(
    int afterCommandId,
    int objectID,
    float finishTime,
    bool relative)
    : Command(afterCommandId, finishTime),
    m_objectID(objectID),
    m_relative(relative)
{
}

void NodeCommand::bind(const RenderContext& ctx, Node* node)
{
    Command::bind(ctx);
    m_node = node;
}
