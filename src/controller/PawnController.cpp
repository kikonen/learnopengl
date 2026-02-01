#include "PawnController.h"

#include <cmath>
#include <iostream>

#include "asset/Assets.h"

#include "util/glm_util.h"
#include "util/glm_format.h"

#include "gui/Input.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "component/FpsCamera.h"

#include "audio/Source.h"

#include "engine/PrepareContext.h"
#include "engine/InputContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
    const auto walkId = SID("walk_2");
    const auto runId = SID("run_2");
    const auto turnId = SID("turn_1");

    constexpr float EPSILON = 0.00001f;
}

PawnController::PawnController()
    : NodeController(true, false)
{
}

void PawnController::prepare(
    const PrepareContext& ctx,
    model::Node& node)
{
    NodeController::prepare(ctx, node);

    const auto& assets = ctx.getAssets();

    m_nodeHandle = node.toHandle();

    m_speedMoveNormal = assets.cameraMoveNormal;
    m_speedMoveRun = assets.cameraMoveRun;
    m_speedRotateNormal = assets.cameraRotateNormal;
    m_speedRotateRun = assets.cameraRotateRun;

    m_speedMouseSensitivity = assets.cameraMouseSensitivity;
}

bool PawnController::updateWT(
    const UpdateContext& ctx,
    model::Node& node)
{
    const auto& intent = m_moveIntent;
    const auto dt = ctx.getClock().elapsedSecs;

    auto& state = NodeRegistry::get().modifyState(node.getEntityIndex());
    bool changed = false;

    // Smooth towards target velocities using exponential decay: dx/dt = -k*(x - target)
    // Solution: x_new = lerp(x_old, target, 1 - e^(-k*dt))
    // Frame-rate independent: same result whether 1 frame at 32ms or 2 frames at 16ms
    // https://www.youtube.com/watch?v=LSNQuFEDOyQ (Freya Holmér - spring/decay math)
    const float t = 1.f - std::exp(-m_smoothing * dt);

    m_moveState.angularVelocity = std::lerp(m_moveState.angularVelocity, intent.angularVelocity, t);
    m_moveState.moveForward = std::lerp(m_moveState.moveForward, intent.moveForward * intent.moveSpeed.z, t);
    m_moveState.moveRight = std::lerp(m_moveState.moveRight, intent.moveRight * intent.moveSpeed.x, t);
    m_moveState.moveUp = std::lerp(m_moveState.moveUp, intent.moveUp * intent.moveSpeed.y, t);

    const bool actionTurn = std::abs(m_moveState.angularVelocity) > EPSILON;
    const bool hasMovement = std::abs(m_moveState.moveForward) > EPSILON ||
                              std::abs(m_moveState.moveRight) > EPSILON ||
                              std::abs(m_moveState.moveUp) > EPSILON;
    const bool actionWalk = intent.moveForward != 0.f || intent.moveRight != 0.f || intent.moveUp != 0.f;
    const bool actionRun = actionWalk && intent.running;

    // Compute movement using MID-rotation view vectors for orbit-like behavior
    if (hasMovement) {
        const auto viewUp = glm::normalize(state.getViewUp());

        // Apply half rotation to get mid-frame view vectors
        glm::vec3 viewFront = glm::normalize(state.getViewFront());
        glm::vec3 viewRight = glm::normalize(state.getViewRight());
        if (actionTurn) {
            auto halfRot = util::axisRadiansToQuat(viewUp, m_moveState.angularVelocity * dt * 0.5f);
            viewFront = halfRot * viewFront;
            viewRight = halfRot * viewRight;
        }

        glm::vec3 velocity =
            viewFront * m_moveState.moveForward +
            viewRight * m_moveState.moveRight +
            viewUp * m_moveState.moveUp;

        state.setPosition(state.getPosition() + velocity * dt);
        changed = true;
    }

    // Apply full rotation
    if (actionTurn) {
        auto rot = util::axisRadiansToQuat(state.getViewUp(), m_moveState.angularVelocity * dt);
        state.adjustRotation(rot);
        changed = true;
    }

    // Audio in sync with movement
    toggleAudio(&node, actionWalk && !actionRun, actionRun, actionTurn && !actionWalk);

    return changed;
}

void PawnController::processInput(
    const InputContext& ctx)
{
    if (!ctx.allowKeyboard()) return;

    auto* node = m_nodeHandle.toNode();
    if (!node) return;

    const auto* snapshot = node->getSnapshotRT();
    if (!snapshot) return;

    const auto& input = ctx.getInput();

    glm::vec3 moveSpeed{ m_speedMoveNormal };
    glm::vec3 rotateSpeed{ glm::radians(m_speedRotateNormal) };

    bool runningSpeed = false;

    if (input.isModifierDown(Modifier::SHIFT)) {
        moveSpeed = m_speedMoveRun;
        rotateSpeed = glm::radians(m_speedRotateRun);
        runningSpeed = true;
    }
    if (input.isModifierDown(Modifier::ALT)) {
        moveSpeed *= 4.f;
        rotateSpeed *= 2.f;
        runningSpeed = true;
    }
    if (input.isHighPrecisionMode()) {
        moveSpeed *= 0.125f;
        rotateSpeed *= 0.125f;
        runningSpeed = false;
    }

    // Rotation from keyboard
    float angularVelocity = 0.f;
    if (input.isKeyDown(Key::ROTATE_LEFT)) {
        angularVelocity += rotateSpeed.y;
    }
    if (input.isKeyDown(Key::ROTATE_RIGHT)) {
        angularVelocity -= rotateSpeed.y;
    }

    // Rotation from mouse
    if (input.isMouseCaptured()) {
        const float maxMouseSpeed = 500.f;
        const float maxAngularSpeed = std::numbers::pi_v<float> * 8.f;
        const float x = input.mouseRelativeX;

        if (std::abs(x) > EPSILON ) {
            float mouseAngularVelocity = -x / maxMouseSpeed * maxAngularSpeed;
            if (input.isHighPrecisionMode()) {
                mouseAngularVelocity *= 0.25f;
            }
            angularVelocity += mouseAngularVelocity;
        }
    }

    // Movement intent - direction computed in updateWT after rotation
    MoveIntent intent;
    intent.angularVelocity = angularVelocity;  // Already in rad/s
    intent.moveSpeed = moveSpeed;
    intent.running = runningSpeed;

    if (input.isKeyDown(Key::FORWARD)) {
        intent.moveForward += 1.f;
    }
    if (input.isKeyDown(Key::BACKWARD)) {
        intent.moveForward -= 1.f;
    }
    if (input.isKeyDown(Key::RIGHT)) {
        intent.moveRight += 1.f;
    }
    if (input.isKeyDown(Key::LEFT)) {
        intent.moveRight -= 1.f;
    }
    if (input.isKeyDown(Key::UP)) {
        intent.moveUp += 1.f;
    }
    if (input.isKeyDown(Key::DOWN)) {
        intent.moveUp -= 1.f;
    }

    m_moveIntent = intent;
}

void PawnController::toggleAudio(
    model::Node* node,
    bool actionWalk,
    bool actionRun,
    bool actionTurn)
{
    if (auto* src = node->getAudioSource(walkId); src) {
        src->toggle(actionWalk);
    }

    if (auto* src = node->getAudioSource(runId); src) {
        src->toggle(actionRun);
    }

    if (auto* src = node->getAudioSource(turnId); src) {
        src->toggle(actionTurn);
    }
}
