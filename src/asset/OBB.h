#pragma once

#include <glm/glm.hpp>

#include "AABB.h"

// https://bruop.github.io/frustum_culling/
struct OBB {
    OBB()
    {
    }

    OBB& operator=(const AABB& aabb);

    inline bool within(float a, float x, float b);

    bool inFrustum(
        int projectedLevel,
        const glm::mat4& projectedMatrix,
        int modelLevel,
        const glm::mat4& modelMatrix);

    void prepareProjected(
        int projectedLevel,
        const glm::mat4& projectedMatrix,
        int modelLevel,
        const glm::mat4& modelMatrix);

    void prepareCorners();

    AABB m_aabb;
    glm::vec4 m_corners[8];
    glm::vec4 m_projected[8];

    int m_projectedLevel = -1;
    int m_modelLevel = -1;
};
