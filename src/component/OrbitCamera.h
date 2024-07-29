#pragma once

#include "CameraComponent.h"

struct Snapshot;

class OrbitCamera : public CameraComponent {
public:
    OrbitCamera();
    ~OrbitCamera();

    virtual void updateRT(const UpdateContext& ctx, Node& node) override;

    float getPitchSpeed() const {
        return m_pitchSpeed;
    }

    void setPitchSpeed(float speed) {
        m_pitchSpeed = speed;
    }

    float getYawSpeed() const {
        return m_yawSpeed;
    }

    void setYawSpeed(float speed) {
        m_yawSpeed = speed;
    }

public:
    glm::vec3 m_offset{ 0.f };
    glm::vec3 m_up{ 0, 1, 0 };

    float m_pitchSpeed{ 0.0f };
    float m_yawSpeed{ 0.0f };
};
