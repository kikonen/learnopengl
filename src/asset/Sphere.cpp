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
    : m_volume{ center, radius }
{}

Sphere::Sphere(const glm::vec4& volume) noexcept
    : m_volume{ volume }
{}

Sphere::~Sphere() noexcept = default;

//std::unique_ptr<Sphere> Sphere::clone() const noexcept
//{
//    auto clone = std::make_unique<Sphere>(m_center, m_radius);
//    clone->m_worldCenter = m_worldCenter;
//    clone->m_worldRadius = m_worldRadius;
//    return clone;
//}

std::string Sphere::str() const noexcept
{
    return fmt::format(
        "<SPHERE: {}>",
        m_volume);
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

glm::vec4 Sphere::calculateWorldVolume(
    const glm::vec4& volume,
    const glm::mat4& modelMatrix,
    const glm::vec3& worldPos,
    float maxScale) noexcept
{
    const auto& center = modelMatrix * glm::vec4(volume.x, volume.y, volume.z, 1.f);
    const auto& radius = volume.w * maxScale;

    return { center.x, center.y, center.z, radius };
}
