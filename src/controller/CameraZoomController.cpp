#include "CameraZoomController.h"

#include <numbers>

#include "asset/Assets.h"

#include "gui/Input.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "component/FpsCamera.h"

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

    m_speedZoomNormal = assets.cameraZoomNormal;
    m_speedZoomRun = assets.cameraZoomRun;

    m_speedMouseSensitivity = assets.cameraMouseSensitivity;
}

bool CameraZoomController::updateWT(
    const UpdateContext& ctx,
    Node& node) noexcept
{
    return false;
}

void CameraZoomController::processInput(
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

    auto* fpsCamera = dynamic_cast<FpsCamera*>(node->m_camera.get());
    if (fpsCamera) {
        float pitchSpeed = 0.f;

        if (input->isMouseCaptured()) {
            const int maxMouseSpeed = 500;
            const float maxPitchSpeed = std::numbers::pi_v<float> *8;
            const float y = input->mouseRelativeY;

            if (y != 0.f)
            {
                // Convert to ~[-1.0, 1.0]
                pitchSpeed = y / maxMouseSpeed;
                pitchSpeed *= maxPitchSpeed;

                if (input->isHighPrecisionMode()) {
                    pitchSpeed *= 0.25f;
                }
            }
        }
        fpsCamera->setPitchSpeed(pitchSpeed);
    }
}

void CameraZoomController::onMouseWheel(
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
