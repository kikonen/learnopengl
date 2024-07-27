#pragma once

#include "render/Camera.h"

struct UpdateContext;
class Node;

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

    virtual void updateRT(const UpdateContext& ctx, Node& node) = 0;

public:
    bool m_enabled{ true };
    bool m_default{ false };

    // camera
    // - horiz = x
    // - vert = y
    // target
    // - dist = z
    glm::vec3 m_distance{ 0.f };

protected:
    ki::level_id m_nodeLevel{ 0 };

    render::Camera m_camera;
};
