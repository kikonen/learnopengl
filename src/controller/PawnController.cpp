#include "PawnController.h"

#include "asset/Assets.h"

#include "util/glm_util.h"
#include "util/glm_format.h"

#include "gui/Input.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "component/FpsCamera.h"

#include "script/CommandEngine.h"
#include "script/api/RotateNode.h"
#include "script/api/MoveNode.h"

#include "audio/AudioEngine.h"

#include "engine/PrepareContext.h"
#include "engine/InputContext.h"

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
                cancelPending(m_pendingRotates);
                auto cmdId = script::CommandEngine::get().addCommand(
                    0,
                    script::RotateNode{
                        m_nodeHandle,
                        0.f,
                        true,
                        snapshot.getViewUp(),
                        adjust.y
                    });
                m_pendingRotates.push_back(cmdId);
                //m_node->getState().adjustQuatRotation(util::degreesToQuat(adjust));
                actionTurn = true;
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
            cancelPending(m_pendingMoves);
            auto cmdId = script::CommandEngine::get().addCommand(
                0,
                script::MoveNode{
                    m_nodeHandle,
                    0.f,
                    true,
                    adjust
                });
            m_pendingMoves.push_back(cmdId);

            //glm::vec3 adjust = snapshot.getPosition();
            //m_node->getState().setPosition(pos);
            actionWalk = true;
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
        cancelPending(m_pendingRotates);
        auto cmdId = script::CommandEngine::get().addCommand(
            0,
            script::RotateNode{
                m_nodeHandle,
                0.f,
                true,
                snapshot.getViewUp(),
                adjust.y
            });
        m_pendingRotates.push_back(cmdId);
        //m_node->getState().adjustQuatRotation(util::degreesToQuat(adjust));
        actionTurn = true;
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

void PawnController::cancelPending(std::vector<script::command_id> pending)
{
    for (auto commandId : pending) {
        script::CommandEngine::get().cancelCommand(commandId);
    }
    pending.clear();
}
