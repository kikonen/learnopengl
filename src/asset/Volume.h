#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "asset/Frustum.h"


// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Volume {
    Volume() = default;
    virtual ~Volume() = default;

    virtual std::unique_ptr<Volume> clone() const = 0;

    bool isOnFrustum(const Frustum& frustum) const;

    virtual bool isOnFrustum(
        const Frustum& camFrustum,
        const int modelMatrixLevel,
        const glm::mat4& modelWorldMatrix) const = 0;

    virtual bool isOnOrForwardPlane(const Plane& plan) const = 0;
};
