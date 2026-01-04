#pragma once

#include <memory>
#include <string>

#include "ki/size.h"

#include "SphereVolume.h"


struct Sphere final
{
    Sphere() noexcept = default;
    Sphere(const glm::vec3& center, float radius) noexcept;
    Sphere(const SphereVolume& volume) noexcept;

    ~Sphere() noexcept;

    std::unique_ptr<Sphere> clone() const noexcept;

    std::string str() const noexcept;

    inline const SphereVolume& getVolume() const noexcept {
        return m_volume;
    }

    inline glm::vec3 getCenter() const noexcept {
        return m_volume.getCenter();
    }

    inline float getRadius() const noexcept {
        return m_volume.radius;
    }

    inline bool isOnOrForwardPlane(const Plane& plane) const noexcept
    {
        return plane.getSignedDistanceToPlane(m_volume.getCenter()) >= -m_volume.radius;
    }

    inline bool isOnOrForwardPlane(
        const glm::vec3& center,
        const Plane& plane) const noexcept
    {
        return plane.getSignedDistanceToPlane(center) >= -m_volume.radius;
    }

    //bool isOnFrustum(
    //    const Frustum& frustum) const noexcept;

    inline bool isOnFrustum(
        const Frustum& frustum) const noexcept
    {
        const auto& center = m_volume.getCenter();

        // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
        return isOnOrForwardPlane(center, frustum.nearFace) &&
            isOnOrForwardPlane(center, frustum.farFace) &&
            isOnOrForwardPlane(center, frustum.leftFace) &&
            isOnOrForwardPlane(center, frustum.rightFace) &&
            isOnOrForwardPlane(center, frustum.topFace) &&
            isOnOrForwardPlane(center, frustum.bottomFace);

        //return isOnOrForwardPlane(frustum.nearFace) &&
        //    isOnOrForwardPlane(frustum.farFace);
    };

    static SphereVolume calculateWorldVolume(
        const SphereVolume& localVolume,
        const glm::mat4& modelMatrix,
        const glm::vec3& worldPos,
        float maxScale) noexcept;

private:
    const SphereVolume m_volume;
};

