
#include "Volume.h"

bool Volume::isOnFrustum(
    const Frustum& frustum,
    const glm::mat4& modelMatrix) const
{
    return isOnOrForwardPlane(frustum.leftFace) &&
        isOnOrForwardPlane(frustum.rightFace) &&
        isOnOrForwardPlane(frustum.topFace) &&
        isOnOrForwardPlane(frustum.bottomFace) &&
        isOnOrForwardPlane(frustum.nearFace) &&
        isOnOrForwardPlane(frustum.farFace);
}
