#include "AABB.h"

#include <iostream>

#include <fmt/format.h>

#include "util/glm_format.h"



void AABB::minmax(const glm::vec3& pos)
{
    m_min.x = std::min(m_min.x, pos.x);
    m_min.y = std::min(m_min.y, pos.y);
    m_min.z = std::min(m_min.z, pos.z);

    m_max.x = std::max(m_max.x, pos.x);
    m_max.y = std::max(m_max.y, pos.y);
    m_max.z = std::max(m_max.z, pos.z);
}


void AABB::updateVolume()
{
    //const glm::vec3 center2 = (m_max + m_min) * 0.5f;
    //const float radius2 = glm::length(m_min - m_max) * 0.5f;

    const glm::vec3 center{ 0.f };
    const float radius = std::max(glm::length(m_min), glm::length(m_max));

    //std::cout << fmt::format("avg_center={}, avg_radius={}, max_radius={}\n", center2, radius2, radius);

    m_volume = glm::vec4(center, radius);
}
