#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

#include "AABB.h"

// https://bruop.github.io/frustum_culling/
struct OBB {
    OBB()
    {
    }

    OBB& operator=(const AABB& aabb);

    inline bool within(float a, float x, float b);

    bool inFrustum(
        ki::level_id projectedLevel,
        const glm::mat4& projectedMatrix,
        ki::level_id modelLevel,
        const glm::mat4& modelMatrix);

    void prepareProjected(
        ki::level_id projectedLevel,
        const glm::mat4& projectedMatrix,
        ki::level_id modelLevel,
        const glm::mat4& modelMatrix);

    void prepareCorners();

    AABB m_aabb;
    glm::vec4 m_corners[8];
    glm::vec4 m_projected[8];

    ki::level_id m_projectedLevel{ 0 };
    ki::level_id m_modelLevel{ 0 };
};
