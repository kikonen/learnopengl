#include "OBB.h"

OBB& OBB::operator=(const AABB& aabb)
{
    m_aabb = aabb;
    prepareCorners();
    return *this;
}

inline bool OBB::within(float a, float x, float b) {
    return x >= a && x <= b;
}

bool OBB::inFrustum(
    ki::level_id projectedLevel,
    const glm::mat4& projectedMatrix,
    ki::level_id modelLevel,
    const glm::mat4& modelMatrix)
{
    bool inside = false;

    prepareProjected(projectedLevel, projectedMatrix, modelLevel, modelMatrix);

    const auto count = m_aabb.m_quad ? 8 : 4;

    for (size_t corner_idx = 0; corner_idx < count; corner_idx++) {
        const auto& corner = m_projected[corner_idx];
        // Check vertex against clip space bounds
        inside = inside ||
            within(-corner.w, corner.x, corner.w) &&
            within(-corner.w, corner.y, corner.w) &&
            within(0.0f, corner.z, corner.w);
    }
    return inside;
}


void OBB::prepareProjected(
    ki::level_id projectedLevel,
    const glm::mat4& projectedMatrix,
    ki::level_id modelLevel,
    const glm::mat4& modelMatrix)
{
    if (m_projectedLevel == projectedLevel && m_modelLevel == modelLevel)
        return;

    // Transform vertex
    const auto& mvp = projectedMatrix * modelMatrix;

    m_projected[0] = mvp * m_corners[0];
    m_projected[1] = mvp * m_corners[1];
    m_projected[2] = mvp * m_corners[2];
    m_projected[3] = mvp * m_corners[3];

    if (!m_aabb.m_quad) {
        m_projected[4] = mvp * m_corners[4];
        m_projected[5] = mvp * m_corners[5];
        m_projected[6] = mvp * m_corners[6];
        m_projected[7] = mvp * m_corners[7];
    }

    m_modelLevel = modelLevel;
    m_projectedLevel = projectedLevel;
}


void OBB::prepareCorners()
{
    // Use our min max to define eight corners
    const auto& min = m_aabb.m_min;
    const auto& max = m_aabb.m_max;

    m_corners[0] = { min.x, min.y, min.z, 1.0 }; // x y z
    m_corners[1] = { max.x, min.y, min.z, 1.0 }; // X y z
    m_corners[2] = { min.x, max.y, min.z, 1.0 }; // x Y z
    m_corners[3] = { max.x, max.y, min.z, 1.0 }; // X Y z

    if (!m_aabb.m_quad) {
        m_corners[4] = { min.x, min.y, max.z, 1.0 }; // x y Z
        m_corners[5] = { max.x, min.y, max.z, 1.0 }; // X y Z
        m_corners[6] = { min.x, max.y, max.z, 1.0 }; // x Y Z
        m_corners[7] = { max.x, max.y, max.z, 1.0 }; // X Y Z
    }
}
