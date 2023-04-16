#pragma once

#include <memory>
#include <string>

#include "Volume.h"


struct Sphere final : public Volume
{
    Sphere() noexcept = default;
    Sphere(const glm::vec3& center, float radius) noexcept;
    Sphere(const glm::vec4& volume) noexcept;

    virtual ~Sphere() noexcept = default;

    const std::string str() const noexcept;

    virtual std::unique_ptr<Volume> clone() const noexcept override;

    virtual const glm::vec3& getCenter() const noexcept override {
        return m_center;
    }

    virtual float getRadius() const noexcept override {
        return m_radius;
    }

    bool isOnOrForwardPlane(const Plane& plan) const noexcept override;

    bool isOnFrustum(
        const Frustum& camFrustum,
        const int modelMatrixLevel,
        const glm::mat4& modelWorldMatrix) const noexcept override;

private:
    void updateWorldSphere(
        const int modelMatrixLevel,
        const glm::mat4& modelMatrix) const noexcept;

private:
    glm::vec3 m_center{ 0.f, 0.f, 0.f };
    float m_radius{ 0.f };

    mutable int m_modelMatrixLevel = -1;
    mutable glm::vec3 m_worldCenter{ 0.f, 0.f, 0.f };
    mutable float m_worldRadius{ 0.f };
};

