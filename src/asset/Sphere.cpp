#pragma once

#include <algorithm>

#include <fmt/format.h>

#include "ki/size.h"

#include "util/glm_format.h"

#include "Sphere.h"

namespace {
    const glm::vec3 ZERO{ 0.f };
}

Sphere::Sphere(const glm::vec3& center, float radius) noexcept
    : m_center{ center },
    m_radius{ radius }
{}

Sphere::Sphere(const glm::vec4& worldVolume) noexcept
    : m_center{ worldVolume },
    m_radius{ worldVolume.a },
    m_worldCenter { worldVolume },
    m_worldRadius{ worldVolume.a }
{}

Sphere::~Sphere() noexcept = default;

std::unique_ptr<Sphere> Sphere::clone() const noexcept
{
    auto clone = std::make_unique<Sphere>(m_center, m_radius);
    clone->m_worldCenter = m_worldCenter;
    clone->m_worldRadius = m_worldRadius;
    return clone;
}

const std::string Sphere::str() const noexcept
{
    return fmt::format(
        "<SPHERE: center={}, radius={}, worldCenter={}, worldRadius={}>",
        m_center, m_radius, m_worldCenter, m_worldRadius);
}

//bool Sphere::isOnFrustum(
//    const Frustum& frustum) const noexcept
//{
//    //float dist[] = {
//    //    frustum.topFace.getSignedDistanceToPlane(m_worldCenter),
//    //    frustum.bottomFace.getSignedDistanceToPlane(m_worldCenter),
//    //    frustum.leftFace.getSignedDistanceToPlane(m_worldCenter),
//    //    frustum.rightFace.getSignedDistanceToPlane(m_worldCenter),
//    //    frustum.nearFace.getSignedDistanceToPlane(m_worldCenter),
//    //    frustum.farFace.getSignedDistanceToPlane(m_worldCenter),
//    //};
//    //bool match[] = {
//    //    isOnOrForwardPlane(frustum.topFace),
//    //    isOnOrForwardPlane(frustum.bottomFace),
//    //    isOnOrForwardPlane(frustum.leftFace),
//    //    isOnOrForwardPlane(frustum.rightFace),
//    //    isOnOrForwardPlane(frustum.nearFace),
//    //    isOnOrForwardPlane(frustum.farFace),
//    //};
//
//    //bool visible = isOnOrForwardPlane(frustum.nearFace) &&
//    //    isOnOrForwardPlane(frustum.topFace) &&
//    //    isOnOrForwardPlane(frustum.bottomFace) &&
//    //    isOnOrForwardPlane(frustum.leftFace) &&
//    //    isOnOrForwardPlane(frustum.rightFace) &&
//    //    isOnOrForwardPlane(frustum.farFace);
//
//    //if (!visible)
//    //    int x = 0;
//
//    // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
//    return isOnOrForwardPlane(frustum.nearFace) &&
//        isOnOrForwardPlane(frustum.leftFace) &&
//        isOnOrForwardPlane(frustum.rightFace) &&
//        isOnOrForwardPlane(frustum.topFace) &&
//        isOnOrForwardPlane(frustum.bottomFace) &&
//        isOnOrForwardPlane(frustum.farFace);
//};

void Sphere::updateVolume(
    const ki::level_id matrixLevel,
    const glm::mat4& modelMatrix,
    const glm::vec3& worldPos,
    float maxScale) const noexcept
{
    if (m_modelMatrixLevel == matrixLevel) {
        return;
    }

    if (m_center == ZERO) {
        m_worldCenter = worldPos;
    }
    else {
        m_worldCenter = modelMatrix * glm::vec4(m_center, 1.f);
    }
    m_worldRadius = m_radius * maxScale;

    m_modelMatrixLevel = matrixLevel;
}
