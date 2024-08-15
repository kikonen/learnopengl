#pragma once

#include "NodeController.h"

class PawnController final : public NodeController
{
public:
    PawnController();

    virtual void prepare(
        const PrepareContext& ctx,
        Node& node) override;

    bool updateWT(
        const UpdateContext& ctx,
        Node& node) override;

    virtual void onKey(
        const InputContext& ctx) override;

    virtual void onMouseMove(
        const InputContext& ctx,
        float xoffset,
        float yoffset) override;

private:
    void toggleAudio(
        Node* node,
        bool actionWalk,
        bool actionTurn);

private:
    pool::NodeHandle m_nodeHandle{};

    glm::vec3 m_speedMoveNormal{ 0.f };
    glm::vec3 m_speedMoveRun{ 0.f };
    glm::vec3 m_speedRotateNormal{ 0.f };
    glm::vec3 m_speedRotateRun{ 0.f };

    glm::vec3 m_speedMouseSensitivity{ 0.f };

    glm::vec3 m_velocity{ 0.f };
    float m_angularVelocity{ 0.f };
};
