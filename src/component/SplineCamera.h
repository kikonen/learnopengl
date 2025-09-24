#pragma once

#include "CameraComponent.h"

#include "Spline.h"

struct Snapshot;

class SplineCamera : public CameraComponent {
public:
    SplineCamera();
    ~SplineCamera();

    virtual void updateRT(const UpdateContext& ctx, model::Node& node) override;

    void restart();

public:
    bool m_paused{ false };

    Spline m_path;

    // Amount t changes/sec
    float m_speed;

private:
    // Current control point index and t
    size_t m_index{ 1 };
    float m_t{ 0.f };
};
