#include "CameraController.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "component/Camera.h"


CameraController::CameraController()
{
}

void CameraController::prepare(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    if (!node.m_camera) return;

    m_node = &node;

    m_cameraMoveNormal = assets.cameraMoveNormal;
    m_cameraMoveRun = assets.cameraMoveRun;
    m_cameraRotateNormal = assets.cameraRotateNormal;
    m_cameraRotateRun = assets.cameraRotateRun;
    m_cameraZoomNormal = assets.cameraZoomNormal;
    m_cameraZoomRun = assets.cameraZoomRun;

    m_cameraMouseSensitivity = assets.cameraMouseSensitivity;
}

bool CameraController::update(
    const UpdateContext& ctx,
    Node& node) noexcept
{
    return false;
}

void CameraController::onKey(Input* input, const ki::RenderClock& clock)
{
    if (!m_node) return;
    auto* camera = m_node->m_camera.get();

    const float dt = clock.elapsedSecs;

    glm::vec3 moveSpeed{ m_cameraMoveNormal };
    glm::vec3 rotateSpeed{ m_cameraRotateNormal };
    glm::vec3 zoomSpeed{ m_cameraZoomNormal };

    if (input->isModifierDown(Modifier::SHIFT)) {
        moveSpeed = m_cameraMoveRun;
        rotateSpeed = m_cameraRotateRun;
        zoomSpeed = m_cameraZoomRun;
    }

    {
        if (true) {
            glm::vec3 rotation = m_node->getRotation();

            if (input->isKeyDown(Key::ROTATE_LEFT)) {
                rotation.y += rotateSpeed.y * dt;
            }
            if (input->isKeyDown(Key::ROTATE_RIGHT)) {
                rotation.y -= rotateSpeed.y * dt;
            }

            m_node->setRotation({ rotation.x, rotation.y, rotation.z });
        }

        if (input->isKeyDown(Key::ZOOM_IN)) {
            camera->adjustFov(-zoomSpeed.z * dt);
        }
        if (input->isKeyDown(Key::ZOOM_OUT)) {
            camera->adjustFov(zoomSpeed.z * dt);
        }
    }

    {
        bool changed = false;
        glm::vec3 pos = m_node->getPosition();

        {
            const auto& viewFront = camera->getViewFront();

            if (input->isKeyDown(Key::FORWARD)) {
                pos += viewFront * dt * moveSpeed.z;
                changed = true;
            }
            if (input->isKeyDown(Key::BACKWARD)) {
                pos -= viewFront * dt * moveSpeed.z;
                changed = true;
            }
        }

        {
            const auto& viewRight = camera->getViewRight();

            if (input->isKeyDown(Key::LEFT)) {
                pos -= viewRight * dt * moveSpeed.x;
                changed = true;
            }
            if (input->isKeyDown(Key::RIGHT)) {
                pos += viewRight * dt * moveSpeed.x;
                changed = true;
            }
        }

        {
            const auto& viewUp = camera->getViewUp();

            if (input->isKeyDown(Key::UP)) {
                pos += viewUp * dt * moveSpeed.y;
                changed = true;
            }
            if (input->isKeyDown(Key::DOWN)) {
                pos -= viewUp * dt * moveSpeed.y;
                changed = true;
            }

        }

        if (changed) {
            m_node->setPosition(pos);
        }
    }
}

void CameraController::onMouseMove(Input* input, float xoffset, float yoffset)
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

void CameraController::onMouseScroll(Input* input, float xoffset, float yoffset)
{
    if (!m_node) return;
    auto* camera = m_node->m_camera.get();

    auto adjustment = m_cameraMouseSensitivity.z * yoffset;

    camera->adjustFov(static_cast<float>(-adjustment));
}
