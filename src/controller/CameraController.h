#pragma once

#include "NodeController.h"


class Camera;

class CameraController final : public NodeController
{
public:
    CameraController();

    virtual void prepare(
        const Assets& assets,
        EntityRegistry& entityRegistry,
        Node& node) override;

    bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) noexcept override;

    virtual void onKey(Input* input, const ki::RenderClock& clock) override;
    virtual void onMouseMove(Input* input, double xoffset, double yoffset) override;
    virtual void onMouseScroll(Input* input, double xoffset, double yoffset) override;

private:
    Node* m_node{ nullptr };

    float m_moveStep = 10.0f;
    float m_rotateStep = 30.f;
    float m_zoomStep = 20.0f;
    float m_mouseSensitivity = 0.1f;
};
