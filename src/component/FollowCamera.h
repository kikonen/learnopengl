#pragma once

#include "CameraComponent.h"

struct Snapshot;

class FollowCamera : public CameraComponent {
public:
    FollowCamera();
    ~FollowCamera();

    virtual void updateRT(const UpdateContext& ctx, Node& node) override;

private:
    glm::vec3 calculateCameraPos(const Snapshot& snapshot);
};
