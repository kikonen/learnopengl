#pragma once

#include "NodeController.h"


class PawnController final : public NodeController
{
public:
    PawnController();

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node) override;

    virtual void onKey(Input* input, const ki::RenderClock& clock) override;
    virtual void onMouseMove(Input* input, float xoffset, float yoffset) override;

private:
    Node* m_node{ nullptr };

    glm::vec3 m_speedMoveNormal{ 0.f };
    glm::vec3 m_speedMoveRun{ 0.f };
    glm::vec3 m_speedRotateNormal{ 0.f };
    glm::vec3 m_speedRotateRun{ 0.f };

    glm::vec3 m_speedMouseSensitivity{ 0.f };
};
