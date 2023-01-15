#pragma once

#include "glm/glm.hpp"


// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Plane {
    // unit vector
    glm::vec3 m_normal = { 0.f, 1.f, 0.f };

    // distance from origin to the nearest point in the plan
    float m_distance = 0.f;

    Plane() noexcept = default;

    Plane(const glm::vec3& p1, const glm::vec3& normal) noexcept
        : m_normal(glm::normalize(normal)),
        m_distance(glm::dot(m_normal, p1))
    {}

    float getSignedDistanceToPlane(const glm::vec3& point) const noexcept
    {
        return glm::dot(m_normal, point) - m_distance;
        }
};
