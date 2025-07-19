#pragma once

#include <memory>
#include <string>

#include "ki/size.h"

#include "Volume.h"


struct Sphere final
{
    Sphere() noexcept = default;
    Sphere(const glm::vec3& center, float radius) noexcept;
    Sphere(const glm::vec4& volume) noexcept;

    ~Sphere() noexcept;

    std::unique_ptr<Sphere> clone() const noexcept;

    std::string str() const noexcept;

    inline const glm::vec4& getVolume() const noexcept {
        return m_volume;
    }

    inline glm::vec3 getCenter() const noexcept {
        return m_volume;
    }

    inline float getRadius() const noexcept {
        return m_volume.w;
    }

    inline bool isOnOrForwardPlane(const Plane& plane) const noexcept
    {
        return plane.getSignedDistanceToPlane(m_volume) >= -m_volume.w;
    }

    //bool isOnFrustum(
    //    const Frustum& frustum) const noexcept;

    inline bool isOnFrustum(
        const Frustum& frustum) const noexcept
    {
        // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
        return isOnOrForwardPlane(frustum.nearFace) &&
            isOnOrForwardPlane(frustum.farFace) &&
            isOnOrForwardPlane(frustum.leftFace) &&
            isOnOrForwardPlane(frustum.rightFace) &&
            isOnOrForwardPlane(frustum.topFace) &&
            isOnOrForwardPlane(frustum.bottomFace);

        //return isOnOrForwardPlane(frustum.nearFace) &&
        //    isOnOrForwardPlane(frustum.farFace);
    };

    static glm::vec4 calculateWorldVolume(
        const glm::vec4& volume,
        const glm::mat4& modelMatrix,
        const glm::vec3& worldPos,
        float maxScale) noexcept;

private:
    const glm::vec4 m_volume;
};

