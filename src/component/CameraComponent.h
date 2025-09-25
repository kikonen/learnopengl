#pragma once

#include "render/Camera.h"

namespace model
{
    class Node;
    struct Snapshot;
}

struct UpdateContext;

class CameraComponent
{
public:
    CameraComponent();
    ~CameraComponent();

    bool isEnabled() const noexcept {
        return m_enabled;
    }

    bool isDefault() const noexcept {
        return m_default;
    }

    render::Camera& getCamera()
    {
        return m_camera;
    }

    virtual void updateRT(const UpdateContext& ctx, model::Node& node) = 0;

    virtual void snapToIdeal(const model::Snapshot& snapshot) {};

public:
    bool m_enabled{ true };
    bool m_default{ false };

protected:
    ki::level_id m_nodeLevel{ 0 };

    render::Camera m_camera;
};
