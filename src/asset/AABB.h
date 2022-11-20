#pragma once

#include <glm/glm.hpp>

// https://bruop.github.io/frustum_culling/
struct AABB final
{
    AABB() noexcept = default;
    AABB(const AABB& aabb) noexcept;
    AABB& operator=(const AABB& aabb) noexcept;

    AABB(const glm::vec3& min, const glm::vec3& max, bool quad) noexcept;

    ~AABB() noexcept = default;

public:
    glm::vec3 m_min;
    glm::vec3 m_max;
    bool m_quad;
};
