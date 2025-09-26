#pragma once

#include <glm/glm.hpp>

#include "util/glm_util.h"

#include "AABB.h"

class AABBBuilder final
{
public:
    AABBBuilder() noexcept
        : m_min{ glm::vec3(std::numeric_limits<float>::max()) },
        m_max{ glm::vec3(std::numeric_limits<float>::min()) }
    {
    }

    void minmax(const glm::vec3& pos)
    {
        util::minmax(pos, m_min, m_max);
    }

    void minmax(const AABBBuilder& o)
    {
        minmax(o.m_min);
        minmax(o.m_max);
    }

    void minmax(const AABB& o)
    {
        minmax(o.getMin());
        minmax(o.getMax());
    }

    AABB toAABB() const noexcept
    {
        glm::vec3 center = (m_max + m_min) * 0.5f;

        glm::vec3 halfExtends{
            std::abs(m_max.x - m_min.x) * 0.5f,
            std::abs(m_max.y - m_min.y) * 0.5f,
            std::abs(m_max.z - m_min.z) * 0.5f
        };

        return { center, halfExtends };
    }

private:
    glm::vec3 m_min{ 0.f };
    glm::vec3 m_max{ 0.f };
};
