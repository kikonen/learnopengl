#include "CameraZoomController.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "component/Camera.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


CameraZoomController::CameraZoomController()
{
}

void CameraZoomController::prepare(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    if (!node.m_camera) return;

    m_registry = registry;

    m_node = &node;

    m_cameraZoomNormal = assets.cameraZoomNormal;
    m_cameraZoomRun = assets.cameraZoomRun;

    m_cameraMouseSensitivity = assets.cameraMouseSensitivity;
}

bool CameraZoomController::update(
    const UpdateContext& ctx,
    Node& node) noexcept
{
    return false;
}

void CameraZoomController::onKey(Input* input, const ki::RenderClock& clock)
{
    if (!m_node) return;
    auto* camera = m_node->m_camera.get();
    const float dt = clock.elapsedSecs;

    glm::vec3 zoomSpeed{ m_cameraZoomNormal };


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
            Node* nextCamera = m_registry->m_nodeRegistry->getNextCamera(m_node, offset);

            // NOTE KI null == default camera
            event::Event evt { event::Type::camera_activate };
            evt.body.node.target = nextCamera;
            m_registry->m_dispatcher->send(evt);
        }
    } else {

        if (input->isModifierDown(Modifier::SHIFT)) {
            zoomSpeed = m_cameraZoomRun;
        }

        if (input->isKeyDown(Key::ZOOM_IN)) {
            camera->adjustFov(-zoomSpeed.z * dt);
        }
        if (input->isKeyDown(Key::ZOOM_OUT)) {
            camera->adjustFov(zoomSpeed.z * dt);
        }
    }
}

void CameraZoomController::onMouseMove(Input* input, float xoffset, float yoffset)
{
    if (!m_node) return;

    bool changed = false;
    const float MAX_ANGLE = 89.f;

    glm::vec3 rotation = m_node->getRotation();

    if (xoffset != 0) {
        auto yaw = rotation.y - m_cameraMouseSensitivity.x * xoffset;

        rotation.y = static_cast<float>(yaw);
        changed = true;
    }

    if (yoffset != 0) {
        auto pitch = rotation.x + m_cameraMouseSensitivity.y * yoffset;

        if (pitch < -MAX_ANGLE) {
            pitch = -MAX_ANGLE;
        }
        if (pitch > MAX_ANGLE) {
            pitch = MAX_ANGLE;
        }

        rotation.x = static_cast<float>(pitch);
        changed = true;
    }

    if (changed) {
        m_node->setRotation(rotation);
    }
}

void CameraZoomController::onMouseScroll(Input* input, float xoffset, float yoffset)
{
    if (!m_node) return;
    auto* camera = m_node->m_camera.get();

    auto adjustment = m_cameraMouseSensitivity.z * yoffset;

    camera->adjustFov(static_cast<float>(-adjustment));
}
