#pragma once

#include "asset/Plane.h"

// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Frustum {
    Frustum() {};

    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};
