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

    m_speedZoomNormal = assets.cameraZoomNormal;
    m_speedZoomRun = assets.cameraZoomRun;

    m_speedMouseSensitivity = assets.cameraMouseSensitivity;
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
            Node* nextCamera = m_registry->m_nodeRegistry->getNextCamera(m_node, offset);

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

void CameraZoomController::onMouseMove(Input* input, float xoffset, float yoffset)
{
    if (!m_node) return;

    bool changed = false;
    const float MAX_ANGLE = 89.f;

    glm::vec3 adjust{ 0.f };

    const auto& curr = m_node->getDegreesRotation();

    const auto maxUp = MAX_ANGLE - curr.x;
    const auto maxDown = -MAX_ANGLE - curr.x;

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
        m_node->adjustQuatRotation(util::degreesToQuat(adjust));
    }
}

void CameraZoomController::onMouseScroll(Input* input, float xoffset, float yoffset)
{
    if (!m_node) return;
    auto* camera = m_node->m_camera.get();

    auto adjustment = m_speedMouseSensitivity.z * yoffset;

    camera->adjustFov(static_cast<float>(-adjustment));
}
