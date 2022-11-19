#pragma once

#include <algorithm>

#include "AABB.h"

AABB::AABB(const glm::vec3& min, const glm::vec3& max) noexcept
    : m_min{ min },
    m_max{ max },
    m_worldAABB{ std::make_unique<AABB>() }
{}

AABB::AABB(const AABB& aabb) noexcept
    : AABB(aabb.m_min, aabb.m_max)
{
}

AABB& AABB::operator=(const AABB& aabb) noexcept
{
    m_min = aabb.m_min;
    m_max = aabb.m_max;
    return *this;
}

//bool AABB::isOnOrForwardPlaneNew(const PlaneNew& plane) const noexcept
//{
//    PlaneNew copy = plane;
//    copy.normalize();
//    const auto dist = plane.distanceToPoint(m_center);
//    const auto distCopy = copy.distanceToPoint(m_center);
//    return -dist > m_radius;
//}
//
//bool AABB::isOnFrustumNew(
//    const FrustumNew& frustum,
//    const int modelMatrixLevel,
//    const glm::mat4& modelWorldMatrix,
//    const int projectedMatrixLevel,
//    const glm::mat4& projectedMatrix) const noexcept
//{
//    updateWorldAABB(modelMatrixLevel, modelWorldMatrix, projectedMatrixLevel, projectedMatrix);
//
//    const auto& worldAABB = *m_worldAABB.get();
//
//    // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
//    return worldAABB.isOnOrForwardPlaneNew(frustum.near) &&
//        worldAABB.isOnOrForwardPlaneNew(frustum.top) &&
//        worldAABB.isOnOrForwardPlaneNew(frustum.bottom) &&
//        worldAABB.isOnOrForwardPlaneNew(frustum.left) &&
//        worldAABB.isOnOrForwardPlaneNew(frustum.right) &&
//        worldAABB.isOnOrForwardPlaneNew(frustum.far);
//};

void AABB::updateWorldAABB(
    const int modelMatrixLevel,
    const glm::mat4& modelWorldMatrix,
    const int projectedMatrixLevel,
    const glm::mat4& projectedMatrix) const noexcept
{
    if (m_modelMatrixLevel == modelMatrixLevel && m_projectedMatrixLevel == projectedMatrixLevel) {
        return;
    }

    // Get world scale thanks to our transform
    // http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/
    float sx = modelWorldMatrix[0][0];
    float sy = modelWorldMatrix[1][1];
    float sz = modelWorldMatrix[2][2];

    // To wrap correctly our shape, we need the maximum scale scalar.
    const float maxScale = std::max(std::max(sx, sy), sz);

    // https://stackoverflow.com/questions/8115352/glmperspective-explanation
    // Get our world center with process it with the world model matrix of our transform
    //auto wp = modelWorldMatrix * glm::vec4(m_center, 1.0);
    //auto pp = projectedMatrix * wp;
    //m_worldAABB->m_center = { pp.x / pp.w, pp.y / pp.w , pp.z / pp.w };

    //// Max scale is assuming for the diameter. So, we need the half to apply it to our radius
    //m_worldAABB->m_radius = m_radius * maxScale * 0.5f;

    m_modelMatrixLevel = modelMatrixLevel;

    m_projectedMatrixLevel = projectedMatrixLevel;
}
