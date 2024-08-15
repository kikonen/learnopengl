#include "PawnController.h"

#include <iostream>

#include "asset/Assets.h"

#include "util/glm_util.h"
#include "util/glm_format.h"

#include "gui/Input.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "component/FpsCamera.h"

#include "audio/AudioEngine.h"

#include "engine/PrepareContext.h"
#include "engine/InputContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"


PawnController::PawnController()
{
}

void PawnController::prepare(
    const PrepareContext& ctx,
    Node& node)
{
    NodeController::prepare(ctx, node);

    const auto& assets = ctx.m_assets;

    m_nodeHandle = node.toHandle();

    m_speedMoveNormal = assets.cameraMoveNormal;
    m_speedMoveRun = assets.cameraMoveRun;
    m_speedRotateNormal = assets.cameraRotateNormal;
    m_speedRotateRun = assets.cameraRotateRun;

    m_speedMouseSensitivity = assets.cameraMouseSensitivity;
}

bool PawnController::updateWT(
    const UpdateContext& ctx,
    Node& node)
{
    const auto dt = ctx.m_clock.elapsedSecs;
    auto& state = node.modifyState();
    bool changed = false;

    if (m_angularVelocity != 0.f) {
        auto rot = util::axisDegreesToQuat(state.getViewUp(), m_angularVelocity * dt);
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

void PawnController::onKey(
    const InputContext& ctx)
{
    if (!ctx.allowKeyboard()) return;

    auto* node = m_nodeHandle.toNode();
    if (!node) return;

    const auto* input = ctx.m_input;

    const float dt = ctx.m_clock.elapsedSecs;

    const auto& snapshot = node->getActiveSnapshot(ctx.m_registry);
    const auto& viewUp = glm::normalize(snapshot.getViewUp());

    glm::vec3 moveSpeed{ m_speedMoveNormal };
    glm::vec3 rotateSpeed{ m_speedRotateNormal };

    if (input->isModifierDown(Modifier::SHIFT)) {
        moveSpeed = m_speedMoveRun;
        rotateSpeed = m_speedRotateRun;

    }
    if (input->isModifierDown(Modifier::ALT)) {
        moveSpeed *= 10;
        rotateSpeed *= 2;
    }
    if (input->isModifierDown(Modifier::CONTROL)) {
        moveSpeed *= 0.1;
        rotateSpeed *= 0.25;
    }

    bool actionWalk = false;
    bool actionTurn = false;

    {
        if (true) {
            bool changed = false;
            glm::vec3 adjust{ 0.f };

            if (input->isKeyDown(Key::ROTATE_LEFT)) {
                adjust.y += rotateSpeed.y * dt;
                changed = true;
            }
            if (input->isKeyDown(Key::ROTATE_RIGHT)) {
                adjust.y += -rotateSpeed.y * dt;
                changed = true;
            }

            if (changed) {
                m_angularVelocity = adjust.y / dt;
                actionTurn = true;
            }
            else {
                m_angularVelocity = 0.f;
            }
        }
    }

    {
        bool changed = false;
        glm::vec3 adjust{ 0.f };

        {
            const auto& viewFront = glm::normalize(snapshot.getViewFront());

            if (input->isKeyDown(Key::FORWARD)) {
                adjust += viewFront * dt * moveSpeed.z;
                changed = true;
            }
            if (input->isKeyDown(Key::BACKWARD)) {
                adjust -= viewFront * dt * moveSpeed.z;
                changed = true;
            }
        }

        {
            const auto& viewRight = glm::normalize(snapshot.getViewRight());

            if (input->isKeyDown(Key::LEFT)) {
                adjust -= viewRight * dt * moveSpeed.x;
                changed = true;
            }
            if (input->isKeyDown(Key::RIGHT)) {
                adjust += viewRight * dt * moveSpeed.x;
                changed = true;
            }
        }

        {
            if (input->isKeyDown(Key::UP)) {
                adjust += viewUp * dt * moveSpeed.y;
                changed = true;
            }
            if (input->isKeyDown(Key::DOWN)) {
                adjust -= viewUp * dt * moveSpeed.y;
                changed = true;
            }

        }

        if (changed) {
            m_velocity = adjust / dt;
            actionWalk = true;
        } else{
            m_velocity = { 0.f, 0.f, 0.f };
        }
    }

    if (actionWalk || actionTurn) {
        // HACK KI try to avoid drifing camea due to camera pitch stuck into some value
        auto* fpsCamera = dynamic_cast<FpsCamera*>(node->m_camera.get());
        if (fpsCamera) {
            fpsCamera->setPitchSpeed(0.f);
        }
    }

    toggleAudio(node, actionWalk, actionTurn);
}

void PawnController::onMouseMove(
    const InputContext& ctx,
    float xoffset,
    float yoffset)
{
    if (!ctx.allowMouse()) return;

    auto* node = m_nodeHandle.toNode();
    if (!node) return;

    bool changed = false;

    const auto& snapshot = node->getActiveSnapshot(ctx.m_registry);

    glm::vec3 adjust{ 0.f };

    bool actionWalk = false;
    bool actionTurn = false;

    if (xoffset != 0) {
        auto yaw = -m_speedMouseSensitivity.x * xoffset;

        adjust.y = static_cast<float>(yaw);
        changed = true;
    }

    if (changed) {
        m_angularVelocity = adjust.y;
        actionTurn = true;
    }
    else {
        m_angularVelocity = 0.f;
    }

    toggleAudio(node, actionWalk, actionTurn);
}

void PawnController::toggleAudio(
    Node* node,
    bool actionWalk,
    bool actionTurn)
{
    auto& ae = audio::AudioEngine::get();
    const auto walkId = node->m_audioSourceIds[1];
    const auto turnId = node->m_audioSourceIds[2];

    ae.toggleSource(walkId, actionWalk);
    ae.toggleSource(turnId, actionTurn && !actionWalk);
}
