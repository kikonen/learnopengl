#pragma once

#include "NodeController.h"


class Camera;

class CameraZoomController final : public NodeController
{
public:
    CameraZoomController();

    virtual void prepare(
        const PrepareContext& ctx,
        Node& node) override;

    virtual void onKey(
        const InputContext& ctx) override;

    virtual void onMouseMove(
        const InputContext& ctx,
        float xoffset,
        float yoffset) override;

    virtual void onMouseScroll(
        const InputContext& ctx,
        float xoffset,
        float yoffset) override;

private:

    pool::NodeHandle m_nodeHandle{};

    bool m_cameraSwitchDown{ false };

    glm::vec3 m_speedZoomNormal{ 0.f };
    glm::vec3 m_speedZoomRun{ 0.f };

    glm::vec3 m_speedMouseSensitivity{ 0.f };
};
