#include "OBB.h"

inline bool OBB::within(float a, float x, float b) {
    return x >= a && x <= b;
}

bool OBB::inFrustum(
    int projectedLevel,
    const glm::mat4& projectedMatrix,
    int modelLevel,
    const glm::mat4& modelMatrix)
{
    bool inside = false;

    prepare(projectedLevel, projectedMatrix, modelLevel, modelMatrix);

    for (size_t corner_idx = 0; corner_idx < 8; corner_idx++) {
        const auto& corner = m_corners[corner_idx];
        // Check vertex against clip space bounds
        inside = inside ||
            within(-corner.w, corner.x, corner.w) &&
            within(-corner.w, corner.y, corner.w) &&
            within(0.0f, corner.z, corner.w);
    }
    return inside;
}


void OBB::prepare(
    int projectedLevel,
    const glm::mat4& projectedMatrix,
    int modelLevel,
    const glm::mat4& modelMatrix)
{
    if (m_projectedLevel == projectedLevel && m_modelLevel == modelLevel) return;

    // Transform vertex
    const auto& mvp = projectedMatrix * modelMatrix;

    // Use our min max to define eight corners
    const auto& min = m_aabb.m_min;
    const auto& max = m_aabb.m_max;

    m_corners[0] = mvp * glm::vec4{ min.x, min.y, min.z, 1.0 }; // x y z
    m_corners[1] = mvp * glm::vec4{ max.x, min.y, min.z, 1.0 }; // X y z
    m_corners[2] = mvp * glm::vec4{ min.x, max.y, min.z, 1.0 }; // x Y z
    m_corners[3] = mvp * glm::vec4{ max.x, max.y, min.z, 1.0 }; // X Y z

    m_corners[4] = mvp * glm::vec4{ min.x, min.y, max.z, 1.0 }; // x y Z
    m_corners[5] = mvp * glm::vec4{ max.x, min.y, max.z, 1.0 }; // X y Z
    m_corners[6] = mvp * glm::vec4{ min.x, max.y, max.z, 1.0 }; // x Y Z
    m_corners[7] = mvp * glm::vec4{ max.x, max.y, max.z, 1.0 }; // X Y Z

    m_modelLevel = modelLevel;
    m_projectedLevel = projectedLevel;
}
