#pragma once

#include <string>
#include <array>

#include "asset/Plane.h"

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Frustum {
    Frustum() noexcept = default;

    std::string str() const noexcept;

    const std::array<glm::vec4, 6> getPlanes() const noexcept {
        return {
            nearFace.getPlane(),
            leftFace.getPlane(),
            rightFace.getPlane(),
            topFace.getPlane(),
            bottomFace.getPlane(),
            farFace.getPlane(),
        };
    }

    Plane topFace;
    Plane bottomFace;

    Plane leftFace;
    Plane rightFace;

    Plane nearFace;
    Plane farFace;
};
