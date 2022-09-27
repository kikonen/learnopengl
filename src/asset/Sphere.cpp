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
    // Get global scale thanks to our transform
    const glm::vec3 globalScale = {
        glm::length(modelMatrix[0]),
        glm::length(modelMatrix[1]),
        glm::length(modelMatrix[2]) };

    // Get our global center with process it with the global model matrix of our transform
    const glm::vec3 globalCenter{ modelMatrix * glm::vec4(center, 1.f) };

    // To wrap correctly our shape, we need the maximum scale scalar.
    const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

    // Max scale is assuming for the diameter. So, we need the half to apply it to our radius
    Sphere globalSphere(globalCenter, radius * (maxScale * 0.5f));

    // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
    bool left = globalSphere.isOnOrForwardPlane(frustum.leftFace);
    bool right = globalSphere.isOnOrForwardPlane(frustum.rightFace);
    bool far = globalSphere.isOnOrForwardPlane(frustum.farFace);
    bool near = globalSphere.isOnOrForwardPlane(frustum.nearFace);
    bool top = globalSphere.isOnOrForwardPlane(frustum.topFace);
    bool bottom = globalSphere.isOnOrForwardPlane(frustum.bottomFace);

    return globalSphere.isOnOrForwardPlane(frustum.leftFace) &&
        globalSphere.isOnOrForwardPlane(frustum.rightFace) &&
        globalSphere.isOnOrForwardPlane(frustum.farFace) &&
        globalSphere.isOnOrForwardPlane(frustum.nearFace) &&
        globalSphere.isOnOrForwardPlane(frustum.topFace) &&
        globalSphere.isOnOrForwardPlane(frustum.bottomFace);
};
