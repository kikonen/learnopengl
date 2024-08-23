#pragma once

#include "NodeController.h"

#include "ki/size.h"

#include "pool/NodeHandle.h"


class CameraZoomController final : public NodeController
{
public:
    CameraZoomController();

    virtual bool updateWT(
        const UpdateContext& ctx,
        Node& node) noexcept override;

    virtual void prepare(
        const PrepareContext& ctx,
        Node& node) override;

    virtual void processInput(
        const InputContext& ctx) override;

    virtual void onMouseWheel(
        const InputContext& ctx,
        float xoffset,
        float yoffset) override;

private:
    pool::NodeHandle m_nodeHandle{};

    ki::level_id m_nodeMatrixLevel{ 0 };
    ki::level_id m_targetMatrixLevel{ 0 };

    bool m_cameraSwitchDown{ false };

    glm::vec3 m_speedZoomNormal{ 0.f };
    glm::vec3 m_speedZoomRun{ 0.f };

    glm::vec3 m_speedMouseSensitivity{ 0.f };

};
