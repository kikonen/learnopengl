#pragma once

#include "glm/glm.hpp"


// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Plane {
    // unit vector
    glm::vec3 normal = { 0.f, 1.f, 0.f };

    // distance from origin to the nearest point in the plan
    float distance = 0.f;

    Plane() = default;

    Plane(const glm::vec3& p1, const glm::vec3& aNormal)
        : normal(glm::normalize(aNormal)),
        distance(glm::dot(normal, p1))
    {}

    float getSignedDistanceToPlane(const glm::vec3& point) const
    {
        return glm::dot(normal, point) - distance;
    }
};
