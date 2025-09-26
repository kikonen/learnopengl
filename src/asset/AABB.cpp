#include "AABB.h"

#include "util/glm_util.h"


glm::vec4 AABB::toVolume() const noexcept
{
    const float radius = glm::length(getMin() - getMax()) * 0.5f;

    return glm::vec4(m_center, radius);
}

glm::vec3 AABB::getMin() const noexcept
{
    return m_center - m_halfExtends;
}

glm::vec3 AABB::getMax() const noexcept
{
    return m_center + m_halfExtends;
}
