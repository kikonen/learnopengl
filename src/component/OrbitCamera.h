#pragma once

#include "CameraComponent.h"

struct Snapshot;

class OrbitCamera : public CameraComponent {
public:
    OrbitCamera();
    ~OrbitCamera();

    virtual void updateRT(const UpdateContext& ctx, model::Node& node) override;

    // @return radians
    float getPitchSpeed() const {
        return m_pitchSpeed;
    }

    // @param speed radians
    void setPitchSpeed(float speed) {
        m_pitchSpeed = speed;
    }

    // @return radians
    float getYawSpeed() const {
        return m_yawSpeed;
    }

    // @param speed radians
    void setYawSpeed(float speed) {
        m_yawSpeed = speed;
    }

public:
    glm::vec3 m_offset{ 0.f };
    glm::vec3 m_up{ 0, 1, 0 };

    // radians
    float m_pitchSpeed{ 0.0f };
    // radians
    float m_yawSpeed{ 0.0f };
};
