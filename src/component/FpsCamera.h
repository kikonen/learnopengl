#pragma once

#include <numbers>

#include "CameraComponent.h"

class FpsCamera : public CameraComponent {
public:
    FpsCamera();
    ~FpsCamera();

    virtual void updateRT(const UpdateContext& ctx, model::Node& node) override;

    virtual void snapToIdeal(const model::Snapshot& snapshot) override;

    // @param pitch radians
    void setPitch(float pitch)
    {
        m_pitch = pitch;
        m_pitch = std::min(std::max(m_pitch, -m_maxPitch), m_maxPitch);
    }

    float getPitch() const
    {
        return m_pitch;
    }

    void setPitchSpeed(float speed)
    {
        m_pitchSpeed = speed;
    }

    float getPitchSpeed() const
    {
        return m_pitchSpeed;
    }

public:
    // Maximum pitch deviation from forward
    float m_maxPitch{ std::numbers::pi_v<float> / 2.1f };

private:
    // Rotation/sec speed of pitch
    float m_pitchSpeed{ 0.f };

    // Current pitch (radians)
    float m_pitch{ 0.f };
};
