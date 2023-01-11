#include "CameraController.h"

#include "model/Node.h"

#include "scene/RenderContext.h"
#include "component/Camera.h"


CameraController::CameraController()
{
}

void CameraController::prepare(
    const Assets& assets,
    EntityRegistry& entityRegistry,
    Node& node)
{
    m_camera = node.m_camera.get();

    m_moveStep = 4.5f;
    m_rotateStep = 15.f;
    m_zoomStep = 20.0f;
    m_mouseSensitivity = 0.1f;
}

bool CameraController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent) noexcept
{
    if (!m_camera) return false;

    auto& camera = m_camera;

    const auto& viewFront = camera->getViewFront();
    const auto& viewUp = camera->getViewUp();
    const auto& pos = camera->getPos();// +(front * 0.1f);
    const auto& rot = camera->getRotation();

    auto nodePos = pos - viewUp * 2.8f + viewFront * 9.f;
    nodePos -= parent->getWorldPos();

    node.setPosition(nodePos);
    node.setRotation({-rot.x, 90 + rot.y, rot.z});

    return true;
}

void CameraController::onKey(Input* input, const ki::RenderClock& clock)
{
    if (!m_camera) return;

    const float dt = clock.elapsedSecs;
    float moveSize = m_moveStep;
    float rotateSize = m_rotateStep;
    if (input->isModifierDown(Modifier::SHIFT)) {
        moveSize *= 3;
        rotateSize *= 3;
    }

    const auto& viewFront = m_camera->getViewFront();
    const auto& viewRight = m_camera->getViewRight();
    const auto& viewUp = m_camera->getViewUp();

    glm::vec3 pos = m_camera->getPos();

    if (input->isKeyDown(Key::FORWARD)) {
        pos += viewFront * dt * moveSize;
    }

    if (input->isKeyDown(Key::BACKWARD)) {
        pos -= viewFront * dt * moveSize;
    }

    if (input->isKeyDown(Key::LEFT)) {
        pos -= viewRight * dt * moveSize;
    }

    if (input->isKeyDown(Key::RIGHT)) {
        pos += viewRight * dt * moveSize;
    }

    if (input->isKeyDown(Key::UP)) {
        pos += viewUp * dt * moveSize;
    }

    if (input->isKeyDown(Key::DOWN)) {
        pos -= viewUp * dt * moveSize;
    }

    m_camera->setPos(pos);

    if (true) {
        glm::vec3 rotation = m_camera->getRotation();

        if (input->isKeyDown(Key::ROTATE_LEFT)) {
            rotation.y  += rotateSize * dt;
        }
        if (input->isKeyDown(Key::ROTATE_RIGHT)) {
            rotation.y -= rotateSize * dt;
        }

        m_camera->setRotation(rotation);
    }


    if (input->isKeyDown(Key::ZOOM_IN)) {
        m_camera->adjustZoom(-m_zoomStep * dt);
    }
    if (input->isKeyDown(Key::ZOOM_OUT)) {
        m_camera->adjustZoom(m_zoomStep * dt);
    }
}

void CameraController::onMouseMove(Input* input, double xoffset, double yoffset)
{
    if (!m_camera) return;

    bool changed = false;
    const float MAX_ANGLE = 89.f;

    glm::vec3 rotation = m_camera->getRotation();

    if (xoffset != 0) {
        auto yaw = rotation.y - m_mouseSensitivity * xoffset;

        rotation.y = yaw;
        changed = true;
    }

    if (yoffset != 0) {
        auto pitch = rotation.x + m_mouseSensitivity * yoffset;

        if (pitch < -MAX_ANGLE) {
            pitch = -MAX_ANGLE;
        }
        if (pitch > MAX_ANGLE) {
            pitch = MAX_ANGLE;
        }

        rotation.x = pitch;
        changed = true;
    }

    if (changed) {
        m_camera->setRotation(rotation);
    }
}

void CameraController::onMouseScroll(Input* input, double xoffset, double yoffset)
{
    if (!m_camera) return;
    m_camera->adjustZoom(-yoffset);
}

void CameraController::updateCamera()
{
    m_camera->updateCamera();
}
