#pragma once

#include <numbers>

#include "CameraComponent.h"

class FpsCamera : public CameraComponent {
public:
    FpsCamera();
    ~FpsCamera();

    virtual void updateRT(const UpdateContext& ctx, Node& node) override;

    virtual void snapToIdeal(const Snapshot& snapshot) override;

    void setPitchSpeed(float speed)
    {
        m_pitchSpeed = speed;
    }

    float getPitchSpeed(float speed)
    {
        return m_pitchSpeed;
    }

public:
    // Maximum pitch deviation from forward
    float m_maxPitch{ std::numbers::pi_v<float> / 3.f };

private:
    // Rotation/sec speed of pitch
    float m_pitchSpeed{ 0.f };

    // Current pitch
    float m_pitch{ 0.f };
};
