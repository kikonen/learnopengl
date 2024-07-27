#include "CameraZoomController.h"

#include "asset/Assets.h"

#include "gui/Input.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"
#include "engine/InputContext.h"
#include "engine/UpdateContext.h"

#include "render/Camera.h"

#include "event/Dispatcher.h"

#include "script/CommandEngine.h"
#include "script/api/RotateNode.h"
#include "script/api/MoveNode.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/NodeSnapshotRegistry.h"



CameraZoomController::CameraZoomController()
{
}

void CameraZoomController::prepare(
    const PrepareContext& ctx,
    Node& node)
{
    NodeController::prepare(ctx, node);
    const auto& assets = ctx.m_assets;

    if (!node.m_camera) return;

    m_nodeHandle = node.toHandle();

    m_targetHandle = pool::NodeHandle::toHandle(m_targetId);

    m_speedZoomNormal = assets.cameraZoomNormal;
    m_speedZoomRun = assets.cameraZoomRun;

    m_speedMouseSensitivity = assets.cameraMouseSensitivity;
}

bool CameraZoomController::updateWT(
    const UpdateContext& ctx,
    Node& node) noexcept
{
    return false;

    Node* targetNode = m_targetHandle.toNode();
    if (!targetNode) return false;

    const auto& state = node.getState();
    const bool nodeChanged = m_nodeMatrixLevel != state.m_matrixLevel;

    const auto& targetState = targetNode->getState();
    const bool targetChanged = m_targetMatrixLevel != targetState.m_matrixLevel;

    if (!(targetChanged || nodeChanged)) return false;

    m_nodeMatrixLevel = state.m_matrixLevel;
    m_targetMatrixLevel = targetState.m_matrixLevel;

    const auto& targetPos = targetState.getWorldPosition();
    const auto& targetFront = targetState.getViewFront();

    const auto& nodePos = targetPos + -targetFront * m_distance;

    // TODO KI resolve node placement

    script::CommandEngine::get().addCommand(
        0,
        script::MoveNode{
            m_nodeHandle,
            0.2f,
            false,
            nodePos
        });

    return true;
}

void CameraZoomController::onKey(
    const InputContext& ctx)
{
    if (!ctx.allowKeyboard()) return;

    auto* node = m_nodeHandle.toNode();
    if (!node) return;

    const auto* input = ctx.m_input;

    auto& camera = node->m_camera.get()->getCamera();
    const float dt = ctx.m_clock.elapsedSecs;

    glm::vec3 zoomSpeed{ m_speedZoomNormal };

    if (input->isModifierDown(Modifier::CONTROL)) {
        int offset = 0;
        if (input->isKeyDown(Key::ZOOM_IN) || input->isKeyDown(Key::ZOOM_OUT)) {
            if (input->isKeyDown(Key::ZOOM_IN)) {
                offset = 1;
            }
            if (input->isKeyDown(Key::ZOOM_OUT)) {
                offset = -1;
            }
        } else {
            m_cameraSwitchDown = false;
        }

        if (m_cameraSwitchDown) {
            offset = 0;
        }

        if (offset != 0) {
            m_cameraSwitchDown = true;
            auto nextCamera = NodeRegistry::get().getNextCameraNode(m_nodeHandle, offset);

            // NOTE KI null == default camera
            event::Event evt { event::Type::camera_activate };
            evt.body.node.target = nextCamera;
            m_registry->m_dispatcherWorker->send(evt);
        }
    } else {

        if (input->isModifierDown(Modifier::SHIFT)) {
            zoomSpeed = m_speedZoomRun;
        }

        if (input->isKeyDown(Key::ZOOM_IN)) {
            camera.adjustFov(-zoomSpeed.z * dt);
        }
        if (input->isKeyDown(Key::ZOOM_OUT)) {
            camera.adjustFov(zoomSpeed.z * dt);
        }
    }
}

void CameraZoomController::onMouseMove(
    const InputContext& ctx,
    float xoffset,
    float yoffset)
{
    if (!ctx.allowMouse()) return;

    auto* node = m_nodeHandle.toNode();
    if (!node) return;

    bool changed = false;
    const float MAX_ANGLE = 89.f;

    glm::vec3 adjust{ 0.f };

    auto& snapshotRegistry = *ctx.m_registry->m_activeSnapshotRegistry;

    const auto& snapshot = snapshotRegistry.getSnapshot(node->m_snapshotIndex);

    const auto& curr = snapshot.getDegreesRotation();
    float currX = curr.x;
    if (currX == 180.f) {
        currX = 0.f;
    }

    const auto maxUp = MAX_ANGLE - currX;
    const auto maxDown = -MAX_ANGLE - currX;

    if (yoffset != 0) {
        auto pitch = m_speedMouseSensitivity.y * yoffset;

        if (pitch < maxDown) {
            pitch = maxDown;
        }
        if (pitch > maxUp) {
            pitch = maxUp;
        }

        adjust.x = static_cast<float>(pitch);
        changed = true;
    }

    if (changed) {
        script::CommandEngine::get().addCommand(
            0,
            script::RotateNode{
                m_nodeHandle,
                0.f,
                true,
                snapshot.getViewRight(),
                -adjust.x
            });
    }
}

void CameraZoomController::onMouseScroll(
    const InputContext& ctx,
    float xoffset,
    float yoffset)
{
    if (!ctx.allowMouse()) return;

    auto* node = m_nodeHandle.toNode();
    if (!node) return;

    auto& camera = node->m_camera.get()->getCamera();

    auto adjustment = m_speedMouseSensitivity.z * yoffset;

    camera.adjustFov(static_cast<float>(-adjustment));
}
