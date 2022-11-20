#pragma once

#include <glm/glm.hpp>

#include "AABB.h"

// https://bruop.github.io/frustum_culling/
struct OBB {
    OBB()
    {
    }

    OBB& operator=(const AABB& aabb)
    {
        m_aabb = aabb;

        return *this;
    }

    inline bool within(float a, float x, float b);

    bool inFrustum(
        int projectedLevel,
        const glm::mat4& projectedMatrix,
        int modelLevel,
        const glm::mat4& modelMatrix);

    void prepare(
        int projectedLevel,
        const glm::mat4& projectedMatrix,
        int modelLevel,
        const glm::mat4& modelMatrix);

    AABB m_aabb;
    glm::vec4 m_corners[8];

    int m_projectedLevel = -1;
    int m_modelLevel = -1;
};
