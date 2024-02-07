#pragma once

#include <memory>
#include <string>

#include "ki/size.h"

#include "Volume.h"


struct Sphere final
{
    Sphere() noexcept = default;
    Sphere(const glm::vec3& center, float radius) noexcept;
    Sphere(const glm::vec4& worldVolume) noexcept;

    ~Sphere() noexcept;

    std::unique_ptr<Sphere> clone() const noexcept;

    std::string str() const noexcept;

    inline const glm::vec4 getVolume() const noexcept {
        return { m_center, m_radius };
    }

    inline const glm::vec4 getWorldVolume() const noexcept {
        return { m_worldCenter, m_worldRadius };
    }

    inline void storeWorldVolume(glm::vec4& volume) const noexcept {
        volume.x = m_worldCenter.x;
        volume.y = m_worldCenter.y;
        volume.z = m_worldCenter.z;
        volume.a = m_worldRadius;
    }

    inline const glm::vec3& getCenter() const noexcept {
        return m_center;
    }

    inline float getRadius() const noexcept {
        return m_radius;
    }

    inline bool isOnOrForwardPlane(const Plane& plane) const noexcept
    {
        return plane.getSignedDistanceToPlane(m_worldCenter) >= -m_worldRadius;
    }

    //bool isOnFrustum(
    //    const Frustum& frustum) const noexcept;

    inline bool isOnFrustum(
        const Frustum& frustum) const noexcept
    {
        // Check Firstly the result that have the most chance to faillure to avoid to call all functions.
        //return isOnOrForwardPlane(frustum.nearFace) &&
        //    isOnOrForwardPlane(frustum.farFace) &&
        //    isOnOrForwardPlane(frustum.leftFace) &&
        //    isOnOrForwardPlane(frustum.rightFace) &&
        //    isOnOrForwardPlane(frustum.topFace) &&
        //    isOnOrForwardPlane(frustum.bottomFace);

        return isOnOrForwardPlane(frustum.nearFace) &&
            isOnOrForwardPlane(frustum.farFace);
    };

    void updateVolume(
        const ki::level_id matrixLevel,
        const glm::mat4& modelMatrix,
        const glm::vec3& worldPos,
        float maxScale) const noexcept;

private:
    glm::vec3 m_center{ 0.f };
    float m_radius{ 0.f };

    mutable ki::level_id m_modelMatrixLevel{ (ki::level_id)-1 };
    mutable glm::vec3 m_worldCenter{ 0.f };
    mutable float m_worldRadius{ 0.f };
};

