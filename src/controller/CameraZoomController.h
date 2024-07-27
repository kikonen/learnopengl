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

public:
    ki::node_id m_targetId{ 0 };

    // Relative to target viewFront & target worldPosition
    glm::vec3 m_direction{ 0.f };
    float m_distance{ 0.f };

private:
    pool::NodeHandle m_nodeHandle{};
    pool::NodeHandle m_targetHandle{};

    ki::level_id m_nodeMatrixLevel{ 0 };
    ki::level_id m_targetMatrixLevel{ 0 };

    bool m_cameraSwitchDown{ false };

    glm::vec3 m_speedZoomNormal{ 0.f };
    glm::vec3 m_speedZoomRun{ 0.f };

    glm::vec3 m_speedMouseSensitivity{ 0.f };

};
