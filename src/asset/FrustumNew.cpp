#include "FrustumNew.h"

namespace {

    enum class Halfspace : std::underlying_type_t<std::byte>
    {
        NEGATIVE,
        ON_PLANE,
        POSITIVE,
    };

    Halfspace ClassifyPoint(const PlaneNew& plane, const glm::vec3& pt)
    {
        float d = plane.x * pt.x + plane.y * pt.y + plane.z * pt.z + plane.w;
        if (d < 0) return Halfspace::NEGATIVE;
        if (d > 0) return Halfspace::POSITIVE;
        return Halfspace::ON_PLANE;
    }
}

void FrustumNew::normalize()
{
    for (int i = 0; i < 6; i++) {
        normalizePlane(m_planes[i]);
    }
}

void FrustumNew::normalizePlane(PlaneNew& plane)
{
    float mag = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
    float im = 1.0f / mag;
    plane.x *= im;
    plane.y *= im;
    plane.z *= im;
    plane.w *= im;
}
