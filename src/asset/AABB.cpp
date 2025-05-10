#include "AABB.h"

#include <iostream>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/glm_util.h"


void AABB::minmax(const glm::vec3& pos)
{
    util::minmax(pos, m_min, m_max);
}

void AABB::minmax(const AABB& o)
{
    minmax(o.m_min);
    minmax(o.m_max);
}

void AABB::updateVolume()
{
    const glm::vec3 center = (m_max + m_min) * 0.5f;
    const float radius = glm::length(m_min - m_max) * 0.5f;

    //const glm::vec3 center{ 0.f };
    //const float radius = std::max(glm::length(m_min), glm::length(m_max));

    //std::cout << fmt::format("avg_center={}, avg_radius={}, max_radius={}\n", center2, radius2, radius);

    m_volume = glm::vec4(center, radius);
}
