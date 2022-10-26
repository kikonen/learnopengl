#pragma once

#include <memory>

#include "Volume.h"


struct Sphere final : public Volume
{
    Sphere() = default;
    Sphere(const glm::vec3& center, float radius);

    virtual ~Sphere() = default;

    virtual std::unique_ptr<Volume> clone() const override final;

    virtual const glm::vec3& getCenter() const override final;
    virtual float getRadius() const override final;

    bool isOnOrForwardPlane(const Plane& plan) const final;

    bool isOnFrustum(
        const Frustum& camFrustum,
        const int modelMatrixLevel,
        const glm::mat4& modelWorldMatrix) const final;

private:
    void updateWorldSphere(
        const int modelMatrixLevel,
        const glm::mat4& modelMatrix) const;

private:
    glm::vec3 m_center{ 0.f, 0.f, 0.f };
    float m_radius{ 0.f };

    mutable int m_modelMatrixLevel = 0;
    mutable std::unique_ptr<Sphere> m_worldSphere;
};

