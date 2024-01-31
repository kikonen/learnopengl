#include "CameraZoomController.h"

#include "asset/Assets.h"

#include "gui/Input.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"
#include "engine/InputContext.h"

#include "component/Camera.h"

#include "event/Dispatcher.h"

#include "script/CommandEngine.h"
#include "script/api/RotateNode.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/SnapshotRegistry.h"


CameraZoomController::CameraZoomController()
{
}

void CameraZoomController::prepare(
    const PrepareContext& ctx,
    Node& node)
{
    NodeController::prepare(ctx, node);
    const auto& assets = Assets::get();

    if (!node.m_camera) return;

    m_nodeHandle = node.toHandle();

    m_speedZoomNormal = assets.cameraZoomNormal;
    m_speedZoomRun = assets.cameraZoomRun;

    m_speedMouseSensitivity = assets.cameraMouseSensitivity;
}

void CameraZoomController::onKey(
    const InputContext& ctx)
{
    auto* node = m_nodeHandle.toNode();
    if (!node) return;

    const auto* input = ctx.m_input;

    auto* camera = node->m_camera.get();
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
            auto nextCamera = m_registry->m_nodeRegistry->getNextCameraNode(m_nodeHandle, offset);

            // NOTE KI null == default camera
            event::Event evt { event::Type::camera_activate };
            evt.body.node.target = nextCamera;
            m_registry->m_dispatcher->send(evt);
        }
    } else {

        if (input->isModifierDown(Modifier::SHIFT)) {
            zoomSpeed = m_speedZoomRun;
        }

        if (input->isKeyDown(Key::ZOOM_IN)) {
            camera->adjustFov(-zoomSpeed.z * dt);
        }
        if (input->isKeyDown(Key::ZOOM_OUT)) {
            camera->adjustFov(zoomSpeed.z * dt);
        }
    }
}

void CameraZoomController::onMouseMove(
    const InputContext& ctx,
    float xoffset,
    float yoffset)
{
    auto* node = m_nodeHandle.toNode();
    if (!node) return;

    bool changed = false;
    const float MAX_ANGLE = 89.f;

    glm::vec3 adjust{ 0.f };

    const auto& snapshot = ctx.m_registry->m_snapshotRegistry->getActiveSnapshot(node->m_snapshotIndex);

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
        m_registry->m_commandEngine->addCommand(
            0,
            script::RotateNode{
                m_nodeHandle.toId(),
                0.f,
                true,
                snapshot.getViewRight(),
                -adjust.x
            });

        //m_node->getTransform().adjustQuatRotation(util::degreesToQuat(adjust));
    }
}

void CameraZoomController::onMouseScroll(
    const InputContext& ctx,
    float xoffset,
    float yoffset)
{
    auto* node = m_nodeHandle.toNode();
    if (!node) return;

    auto* camera = node->m_camera.get();

    auto adjustment = m_speedMouseSensitivity.z * yoffset;

    camera->adjustFov(static_cast<float>(-adjustment));
}
