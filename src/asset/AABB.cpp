#include "AABB.h"


void AABB::minmax(const glm::vec3& pos)
{
    m_min.x = std::min(m_min.x, pos.x);
    m_min.y = std::min(m_min.y, pos.y);
    m_min.z = std::min(m_min.z, pos.z);

    m_max.x = std::max(m_max.x, pos.x);
    m_max.y = std::max(m_max.y, pos.y);
    m_max.z = std::max(m_max.z, pos.z);
}
