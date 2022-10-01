#pragma once

#include <algorithm>

#include "Sphere.h"

bool Sphere::isOnOrForwardPlane(const Plane& plane) const 
{
    return plane.getSignedDistanceToPlane(center) > -radius;
}

bool Sphere::isOnFrustum(
    const Frustum& frustum,
    const glm::mat4& modelMatrix) const 
{
    // Get world scale thanks to our transform
    const glm::vec3 worldScale = {
        glm::length(modelMatrix[0]),
        glm::length(modelMatrix[1]),
        glm::length(modelMatrix[2]) };

    // Get our world center with process it with the world model matrix of our transform
    const glm::vec3 worldCenter{ modelMatrix * glm::vec4(center, 1.f) };

    // To wrap correctly our shape, we need the maximum scale scalar.
    const float maxScale = std::max(std::max(worldScale.x, worldScale.y), worldScale.z);

    // Max scale is assuming for the diameter. So, we need the half to apply it to our radius
    Sphere worldSphere(worldCenter, radius * (maxScale * 0.5f));

    // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
    bool left = worldSphere.isOnOrForwardPlane(frustum.leftFace);
    bool right = worldSphere.isOnOrForwardPlane(frustum.rightFace);
    bool far = worldSphere.isOnOrForwardPlane(frustum.farFace);
    bool near = worldSphere.isOnOrForwardPlane(frustum.nearFace);
    bool top = worldSphere.isOnOrForwardPlane(frustum.topFace);
    bool bottom = worldSphere.isOnOrForwardPlane(frustum.bottomFace);

    return worldSphere.isOnOrForwardPlane(frustum.leftFace) &&
        worldSphere.isOnOrForwardPlane(frustum.rightFace) &&
        worldSphere.isOnOrForwardPlane(frustum.farFace) &&
        worldSphere.isOnOrForwardPlane(frustum.nearFace) &&
        worldSphere.isOnOrForwardPlane(frustum.topFace) &&
        worldSphere.isOnOrForwardPlane(frustum.bottomFace);
};
