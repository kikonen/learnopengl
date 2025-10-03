#include "PawnController.h"

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
    const auto dt = ctx.getClock().elapsedSecs;

    auto& state = NodeRegistry::get().modifyState(node.getEntityIndex());
    bool changed = false;

    float angularVelocity = m_angularVelocity;
    if (angularVelocity != 0.f) {
        auto rot = util::axisRadiansToQuat(state.getViewUp(), angularVelocity * dt);
        state.adjustRotation(rot);
        changed = true;
    }

    if (m_velocity != glm::vec3{ 0.f }) {
        auto adjust = m_velocity * dt;
        state.setPosition(state.getPosition() + adjust);
        changed = true;
    }
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

    const float dt = ctx.getClock().elapsedSecs;

    const auto& viewUp = glm::normalize(snapshot->getViewUp());

    glm::vec3 moveSpeed{ m_speedMoveNormal };
    glm::vec3 rotateSpeed{ glm::radians(m_speedRotateNormal) };

    bool runningSpeed = false;

    if (input.isModifierDown(Modifier::SHIFT)) {
        moveSpeed = m_speedMoveRun;
        rotateSpeed = glm::radians(m_speedRotateRun);
        runningSpeed = true;
    }
    if (input.isModifierDown(Modifier::ALT)) {
        moveSpeed *= 5.f;
        rotateSpeed *= 2.f;
        runningSpeed = true;
    }
    if (input.isHighPrecisionMode()) {
        moveSpeed *= 0.1f;
        rotateSpeed *= 0.25f;
        runningSpeed = false;
    }

    bool actionWalk = false;
    bool actionRun = false;
    bool actionTurn = false;

    float angularVelocity = 0.f;

    {
        if (true) {
            bool changed = false;

            if (input.isKeyDown(Key::ROTATE_LEFT)) {
                angularVelocity += rotateSpeed.y;
                changed = true;
            }
            if (input.isKeyDown(Key::ROTATE_RIGHT)) {
                angularVelocity += -rotateSpeed.y;
                changed = true;
            }

            if (changed) {
                actionTurn = true;
            }
        }
    }

    {
        bool changed = false;
        glm::vec3 adjust{ 0.f };

        {
            const auto& viewFront = glm::normalize(snapshot->getViewFront());

            if (input.isKeyDown(Key::FORWARD)) {
                adjust += viewFront * dt * moveSpeed.z;
                changed = true;
            }
            if (input.isKeyDown(Key::BACKWARD)) {
                adjust -= viewFront * dt * moveSpeed.z;
                changed = true;
            }
        }

        {
            const auto& viewRight = glm::normalize(snapshot->getViewRight());

            if (input.isKeyDown(Key::LEFT)) {
                adjust -= viewRight * dt * moveSpeed.x;
                changed = true;
            }
            if (input.isKeyDown(Key::RIGHT)) {
                adjust += viewRight * dt * moveSpeed.x;
                changed = true;
            }
        }

        {
            if (input.isKeyDown(Key::UP)) {
                adjust += viewUp * dt * moveSpeed.y;
                changed = true;
            }
            if (input.isKeyDown(Key::DOWN)) {
                adjust -= viewUp * dt * moveSpeed.y;
                changed = true;
            }

        }

        if (changed) {
            m_velocity = adjust / dt;
            actionWalk = true;
            actionRun = runningSpeed;
        } else{
            m_velocity = { 0.f, 0.f, 0.f };
        }
    }

    if (input.isMouseCaptured()) {
        const float maxMouseSpeed = 500.f;
        // Rotation/sec at maximum speed
        const float maxAngularSpeed = std::numbers::pi_v<float> * 8.f;
        const float x = input.mouseRelativeX;

        if (x != 0.f) {
            // Convert to ~[-1.0, 1.0]
            float mouseAngularVelocity = -x / maxMouseSpeed;
            // Multiply by rotation/sec
            mouseAngularVelocity *= maxAngularSpeed;

            if (input.isHighPrecisionMode()) {
                mouseAngularVelocity *= 0.25f;
            }

            angularVelocity += mouseAngularVelocity;
            actionTurn = true;
        }
    }

    m_angularVelocity = angularVelocity;

    toggleAudio(node, actionWalk, actionRun, actionTurn);
}

void PawnController::toggleAudio(
    model::Node* node,
    bool actionWalk,
    bool actionRun,
    bool actionTurn)
{
    if (auto* src = node->getAudioSource(walkId); src) {
        src->toggle(actionWalk && !actionRun);
    }

    if (auto* src = node->getAudioSource(runId); src) {
        src->toggle(actionRun);
    }

    if (auto* src = node->getAudioSource(turnId); src) {
        src->toggle(actionTurn && !(actionWalk || actionRun));
    }
}
