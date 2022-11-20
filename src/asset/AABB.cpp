#include "AABB.h"

AABB::AABB(const glm::vec3& min, const glm::vec3& max, bool quad) noexcept
    : m_min{ min },
    m_max{ max },
    m_quad { quad }
{}

AABB::AABB(const AABB& aabb) noexcept
    : AABB(aabb.m_min, aabb.m_max, aabb.m_quad)
{
}

AABB& AABB::operator=(const AABB& aabb) noexcept
{
    m_min = aabb.m_min;
    m_max = aabb.m_max;
    return *this;
}
