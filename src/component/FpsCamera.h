#pragma once

#include "CameraComponent.h"

class FpsCamera : public CameraComponent {
public:
    FpsCamera();
    ~FpsCamera();

    virtual void updateRT(const UpdateContext& ctx, Node& node) override;

    virtual void snapToIdeal(const Snapshot& snapshot) override;

};
