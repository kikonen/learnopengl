#pragma once

#include <string>

#include "asset/Plane.h"

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Frustum {
    Frustum() noexcept = default;

    const std::string str() const noexcept;

    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};
