#pragma once

#include <memory>
#include <string>

#include "Volume.h"


struct Sphere final : public Volume
{
    Sphere() noexcept = default;
    Sphere(const glm::vec3& center, float radius) noexcept;
    Sphere(const glm::vec4& worldVolume) noexcept;

    virtual ~Sphere() noexcept = default;

    virtual std::unique_ptr<Volume> clone() const noexcept override;

    const std::string str() const noexcept;

    inline const glm::vec4 getVolume() const noexcept {
        return { m_center, m_radius };
    }

    inline const glm::vec4 getWorldVolume() const noexcept {
        return { m_worldCenter, m_worldRadius };
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

    bool isOnFrustum(const Frustum& frustum) const noexcept override;

    virtual void updateVolume(
        const int matrixLevel,
        const glm::mat4& modelMatrix,
        float maxScale) const noexcept override;

private:
    glm::vec3 m_center{ 0.f, 0.f, 0.f };
    float m_radius{ 0.f };

    mutable int m_modelMatrixLevel = -1;
    mutable glm::vec3 m_worldCenter{ 0.f, 0.f, 0.f };
    mutable float m_worldRadius{ 0.f };
};

