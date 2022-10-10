
#include "command/Command.h"


Command::Command(
    int objectID,
    float secs,
    const glm::vec3& pos)
    : m_objectID(objectID),
    m_secs(secs),
    m_pos(pos)
{

}
