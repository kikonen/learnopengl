#pragma once

#include <algorithm>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "Sphere.h"

Sphere::Sphere(const glm::vec3& center, float radius) noexcept
    : Volume{},
    m_center{ center },
    m_radius{ radius }
{}

Sphere::Sphere(const glm::vec4& volume) noexcept
    : Volume{},
    m_center{ volume },
    m_radius{ volume.a }
{}

const std::string Sphere::str() const noexcept
{
    return fmt::format(
        "<SPHERE: center={}, radius={}, worldCenter={}, worldRadius={}>",
        m_center, m_radius, m_worldCenter, m_worldRadius);
}

std::unique_ptr<Volume> Sphere::clone() const noexcept
{
    return std::make_unique<Sphere>(m_center, m_radius);
}

bool Sphere::isOnOrForwardPlane(const Plane& plane) const noexcept
{
    return plane.getSignedDistanceToPlane(m_worldCenter) > -m_worldRadius;
}

bool Sphere::isOnFrustum(
    const Frustum& frustum,
    const int modelMatrixLevel,
    const glm::mat4& modelWorldMatrix) const noexcept
{
    updateWorldSphere(modelMatrixLevel, modelWorldMatrix);

    //// Check Firstly the result that have the most chance to faillure to avoid to call all functions.
    //const bool left = worldSphere.isOnOrForwardPlane(frustum.leftFace);
    //const bool right = worldSphere.isOnOrForwardPlane(frustum.rightFace);
    //const bool far = worldSphere.isOnOrForwardPlane(frustum.farFace);
    //const bool near = worldSphere.isOnOrForwardPlane(frustum.nearFace);
    //const bool top = worldSphere.isOnOrForwardPlane(frustum.topFace);
    //const bool bottom = worldSphere.isOnOrForwardPlane(frustum.bottomFace);

    //bool nope = worldSphere.isOnOrForwardPlane(frustum.leftFace) &&
    //    worldSphere.isOnOrForwardPlane(frustum.rightFace) &&
    //    worldSphere.isOnOrForwardPlane(frustum.topFace) &&
    //    worldSphere.isOnOrForwardPlane(frustum.bottomFace) &&
    //    worldSphere.isOnOrForwardPlane(frustum.farFace) &&
    //    worldSphere.isOnOrForwardPlane(frustum.nearFace);
    //if (!nope) {
    //    int x = 0;
    //    updateWorldSphere(modelMatrixLevel, modelWorldMatrix);
    //}

    // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
    return isOnOrForwardPlane(frustum.nearFace) &&
        isOnOrForwardPlane(frustum.topFace) &&
        isOnOrForwardPlane(frustum.bottomFace) &&
        isOnOrForwardPlane(frustum.leftFace) &&
        isOnOrForwardPlane(frustum.rightFace) &&
        isOnOrForwardPlane(frustum.farFace);
};

void Sphere::updateWorldSphere(
    const int modelMatrixLevel,
    const glm::mat4& modelWorldMatrix) const noexcept
{
    if (m_modelMatrixLevel == modelMatrixLevel) {
        return;
    }

    // Get world scale thanks to our transform
    // http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/
    float sx = modelWorldMatrix[0][0];
    float sy = modelWorldMatrix[1][1];
    float sz = modelWorldMatrix[2][2];

    // To wrap correctly our shape, we need the maximum scale scalar.
    const float maxScale = std::max(std::max(sx, sy), sz);

    // Get our world center with process it with the world model matrix of our transform
    m_worldCenter = modelWorldMatrix * glm::vec4(m_center, 1.f);

    // Max scale is assuming for the diameter. So, we need the half to apply it to our radius
    m_worldRadius = m_radius * maxScale * 0.5f;

    m_modelMatrixLevel = modelMatrixLevel;
}
