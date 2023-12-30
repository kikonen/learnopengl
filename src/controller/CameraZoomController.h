#pragma once

#include "NodeController.h"


class Camera;

class CameraZoomController final : public NodeController
{
public:
    CameraZoomController();

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node) override;

    bool update(
        const UpdateContext& ctx,
        Node& node) noexcept override;

    virtual void onKey(Input* input, const ki::RenderClock& clock) override;
    virtual void onMouseMove(Input* input, float xoffset, float yoffset) override;
    virtual void onMouseScroll(Input* input, float xoffset, float yoffset) override;

private:

    Node* m_node{ nullptr };

    bool m_cameraSwitchDown{ false };

    glm::vec3 m_speedZoomNormal{ 0.f };
    glm::vec3 m_speedZoomRun{ 0.f };

    glm::vec3 m_speedMouseSensitivity{ 0.f };
};
