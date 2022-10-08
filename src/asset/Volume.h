#pragma once

#include <glm/glm.hpp>

#include "asset/Frustum.h"


// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Volume {
    Volume() = default;
    virtual ~Volume() {}

    bool isOnFrustum(const Frustum& frustum) const;

    virtual bool isOnFrustum(
        const Frustum& frustum,
        const glm::mat4& modelMatrix) const = 0;

    virtual bool isOnOrForwardPlane(const Plane& plan) const = 0;
};
