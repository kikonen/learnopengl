#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "Sphere.h"

// https://bruop.github.io/frustum_culling/
struct AABB final
{
    AABB() noexcept = default;
    AABB(const AABB& aabb) noexcept;
    AABB& operator=(const AABB& aabb) noexcept;

    AABB(const glm::vec3& min, const glm::vec3& max) noexcept;

    virtual ~AABB() noexcept = default;

    //bool isOnOrForwardPlaneNew(const PlaneNew& plane) const noexcept;

    //bool isOnFrustumNew(
    //    const FrustumNew& camFrustum,
    //    const int modelMatrixLevel,
    //    const glm::mat4& modelWorldMatrix,
    //    const int projectedMatrixLevel,
    //    const glm::mat4& projectedMatrix) const noexcept;

private:
    void updateWorldAABB(
        const int modelMatrixLevel,
        const glm::mat4& modelMatrix,
        const int projectedMatrixLevel,
        const glm::mat4& projectedMatrix) const noexcept;

public:
    glm::vec3 m_min;
    glm::vec3 m_max;

private:
    mutable int m_modelMatrixLevel = -1;
    mutable int m_projectedMatrixLevel = -1;
    mutable std::unique_ptr<AABB> m_worldAABB;
};
