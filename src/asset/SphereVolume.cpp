#include "SphereVolume.h"

#include "Plane.h"

namespace
{
    bool isOnOrForwardPlane(
        const glm::vec3& center,
        float radius,
        const Plane& plane) noexcept
    {
        return plane.getSignedDistanceToPlane(center) >= -radius;
    }
}

bool SphereVolume::isOnFrustum(const Frustum& frustum) const noexcept
{
    const auto& center = getCenter();

    // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
    return isOnOrForwardPlane(center, radius, frustum.nearFace) &&
        isOnOrForwardPlane(center, radius, frustum.farFace) &&
        isOnOrForwardPlane(center, radius, frustum.leftFace) &&
        isOnOrForwardPlane(center, radius, frustum.rightFace) &&
        isOnOrForwardPlane(center, radius, frustum.topFace) &&
        isOnOrForwardPlane(center, radius, frustum.bottomFace);
}

SphereVolume SphereVolume::calculateWorldVolume(
    const glm::mat4& modelMatrix,
    float maxScale) const noexcept
{
    const auto& wc = modelMatrix * getPosition();
    const auto wr = radius * maxScale;

    return { wc.x, wc.y, wc.z, wr };
}
