#pragma once

#include "Volume.h"

struct Sphere : public Volume
{
    glm::vec3 center{ 0.f, 0.f, 0.f };
    float radius{ 0.f };

    Sphere() = default;
    virtual ~Sphere() {}

    Sphere(const glm::vec3& center, float radius)
        : Volume{}, center{ center }, radius{ radius }
    {}

    bool isOnOrForwardPlane(const Plane& plan) const final;

    bool isOnFrustum(
        const Frustum& camFrustum,
        const glm::mat4& modelMatrix) const final;
};
