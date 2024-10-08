#pragma once

#include "CameraComponent.h"

struct Snapshot;

class FollowCamera : public CameraComponent {
public:
    FollowCamera();
    ~FollowCamera();

    virtual void updateRT(const UpdateContext& ctx, Node& node) override;

    virtual void snapToIdeal(const Snapshot& snapshot) override;

private:
    glm::vec3 calculateCameraPos(const Snapshot& snapshot);

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
