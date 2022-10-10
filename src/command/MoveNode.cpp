#include "command/MoveNode.h"


MoveNode::MoveNode(
    int objectID,
    float secs,
    const glm::vec3& pos)
    : Command(objectID, secs, pos)
{
}

bool MoveNode::execute(
    const RenderContext& ctx,
    Node& node)
{
    return false;
}
