#pragma once

#include "NodeController.h"


class Camera;

class CameraController final : public NodeController
{
public:
    CameraController();

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node) override;

    bool update(
        const UpdateContext& ctx,
        Node& node) noexcept override;

    virtual void onKey(Input* input, const ki::RenderClock& clock) override;
    virtual void onMouseMove(Input* input, double xoffset, double yoffset) override;
    virtual void onMouseScroll(Input* input, double xoffset, double yoffset) override;

private:
    Node* m_node{ nullptr };

    glm::vec3 m_cameraMoveNormal{ 0.f };
    glm::vec3 m_cameraMoveRun{ 0.f };
    glm::vec3 m_cameraRotateNormal{ 0.f };
    glm::vec3 m_cameraRotateRun{ 0.f };
    glm::vec3 m_cameraZoomNormal{ 0.f };
    glm::vec3 m_cameraZoomRun{ 0.f };

    glm::vec3 m_cameraMouseSensitivity{ 0.f };
};
