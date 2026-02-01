#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "NodeController.h"

class PawnController final : public NodeController
{
public:
    // Movement intent captured from input, applied in updateWT
    struct MoveIntent {
        float angularVelocity{ 0.f };
        float moveForward{ 0.f };   // +1 forward, -1 backward
        float moveRight{ 0.f };     // +1 right, -1 left
        float moveUp{ 0.f };        // +1 up, -1 down
        glm::vec3 moveSpeed{ 0.f };
        bool running{ false };
    };

    // Smoothed state applied each frame
    struct MoveState {
        float angularVelocity{ 0.f };
        float moveForward{ 0.f };
        float moveRight{ 0.f };
        float moveUp{ 0.f };
    };

    PawnController();

    virtual void prepare(
        const PrepareContext& ctx,
        model::Node& node) override;

    bool updateWT(
        const UpdateContext& ctx,
        model::Node& node) override;

    virtual void processInput(
        const InputContext& ctx) override;

private:
    void toggleAudio(
        model::Node* node,
        bool actionWalk,
        bool actionRun,
        bool actionTurn);

private:
    pool::NodeHandle m_nodeHandle{};

    glm::vec3 m_speedMoveNormal{ 0.f };
    glm::vec3 m_speedMoveRun{ 0.f };
    glm::vec3 m_speedRotateNormal{ 0.f };
    glm::vec3 m_speedRotateRun{ 0.f };

    glm::vec3 m_speedMouseSensitivity{ 0.f };

    MoveIntent m_moveIntent{};
    MoveState m_moveState{};

    float m_smoothing{ 12.f };  // Higher = faster response
};
