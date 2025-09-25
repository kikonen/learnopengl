#pragma once

#include "CameraComponent.h"

namespace model
{
    struct Snapshot;
}

class FollowCamera : public CameraComponent {
public:
    FollowCamera();
    ~FollowCamera();

    virtual void updateRT(const UpdateContext& ctx, model::Node& node) override;

    virtual void snapToIdeal(const model::Snapshot& snapshot) override;

private:
    glm::vec3 calculateCameraPos(const model::Snapshot& snapshot);

public:
    // [horiz_offset, vert_offset, target_dist
    glm::vec3 m_distance{ 0.f };

    // Spring constant (higher is more stiff)
    float m_springConstant{ 64.f };

private:
    // Actual position of camera
    glm::vec3 m_actualPos{ 0.f };

    // Velocity of actual camera
    glm::vec3 m_velocity{ 0.f };
};
