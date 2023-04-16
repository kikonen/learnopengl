#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "asset/Frustum.h"


// https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Volume {
    Volume() noexcept = default;
    virtual ~Volume() noexcept = default;

    virtual std::unique_ptr<Volume> clone() const noexcept = 0;

    virtual bool isOnFrustum(const Frustum& frustum) const noexcept = 0;

    virtual void updateVolume(
        const int matrixLevel,
        const glm::mat4& modelMatrix,
        float maxScale) const noexcept = 0;
};
