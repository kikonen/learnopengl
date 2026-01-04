#pragma once

#include <glm/glm.hpp>

#include "asset/SphereVolume.h"

// https://bruop.github.io/frustum_culling/
struct AABB final
{
    AABB() = default;

    AABB(
        const glm::vec3& center,
        const glm::vec3& halfExtends) noexcept
        : m_center{ center },
        m_halfExtends{ halfExtends }
    { }

    AABB(const AABB& o) noexcept
        : AABB(o.m_center, o.m_halfExtends)
    {}

    AABB& operator=(const AABB& o) noexcept
    {
        m_center = o.m_center;
        m_halfExtends = o.m_halfExtends;
        return *this;
    }

    ~AABB() noexcept = default;
    
    SphereVolume toLocalVolume() const noexcept;

    glm::vec3 getMin() const noexcept;
    glm::vec3 getMax() const noexcept;

private:
    glm::vec3 m_center{ 0.f };
    glm::vec3 m_halfExtends{ 0.f };
};
