#pragma once

#include <algorithm>

#include "Sphere.h"

Sphere::Sphere(const glm::vec3& center, float radius)
    : Volume{},
    m_center{ center },
    m_radius{ radius }
{}

std::unique_ptr<Volume> Sphere::clone() const
{
    return std::make_unique<Sphere>(m_center, m_radius);
}

bool Sphere::isOnOrForwardPlane(const Plane& plane) const 
{
    return plane.getSignedDistanceToPlane(m_center) > -m_radius;
}

bool Sphere::isOnFrustum(
    const Frustum& frustum,
    const int modelMatrixLevel,
    const glm::mat4& modelWor�dMatrix) const
{
    updateWorldSphere(modelMatrixLevel, modelWor�dMatrix);

    const auto& worldSphere = *m_worldSphere.get();

    // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
    const bool left = worldSphere.isOnOrForwardPlane(frustum.leftFace);
    const bool right = worldSphere.isOnOrForwardPlane(frustum.rightFace);
    const bool far = worldSphere.isOnOrForwardPlane(frustum.farFace);
    const bool near = worldSphere.isOnOrForwardPlane(frustum.nearFace);
    const bool top = worldSphere.isOnOrForwardPlane(frustum.topFace);
    const bool bottom = worldSphere.isOnOrForwardPlane(frustum.bottomFace);

    return left && right && far && near && top && bottom;
};

void Sphere::updateWorldSphere(
    const int modelMatrixLevel,
    const glm::mat4& modelWorldMatrix) const
{
    if (m_modelMatrixLevel == modelMatrixLevel) {
        return;
    }

    if (!m_worldSphere)
        m_worldSphere = std::make_unique<Sphere>();

    // Get world scale thanks to our transform
    const glm::vec3 worldScale = {
        glm::length(modelWorldMatrix[0]),
        glm::length(modelWorldMatrix[1]),
        glm::length(modelWorldMatrix[2]) };

    // Get our world center with process it with the world model matrix of our transform
    const glm::vec3 worldCenter{ modelWorldMatrix * glm::vec4(m_center, 1.f) };

    // To wrap correctly our shape, we need the maximum scale scalar.
    const float maxScale = std::max(std::max(worldScale.x, worldScale.y), worldScale.z);

    // Max scale is assuming for the diameter. So, we need the half to apply it to our radius
    m_worldSphere->m_center = worldCenter;
    m_worldSphere->m_radius = m_radius * maxScale * 0.5f;

    m_modelMatrixLevel = modelMatrixLevel;
}
