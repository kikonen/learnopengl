
#include "command/Command.h"


Command::Command(
    int objectID,
    float finishTime)
    : m_objectID(objectID),
    m_finishTime(finishTime)
{

}

void Command::bind(const RenderContext& ctx, Node* node)
{
    m_node = node;
    m_elapsedTime = 0.f;
}
