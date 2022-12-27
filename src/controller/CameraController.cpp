#include "CameraController.h"


CameraController::CameraController()
{
}

void CameraController::prepare(const Assets& assets, Node& node)
{
    m_camera = node.m_camera.get();
}

bool CameraController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent) noexcept
{
    if (!m_camera) return false;

    auto& camera = m_camera;

    glm::vec3 viewFront = camera->getViewFront();
    glm::vec3 viewUp = camera->getViewUp();
    glm::vec3 pos = camera->getPos();// +(front * 0.1f);
    glm::vec3 rot = camera->getRotation();

    auto nodePos = pos - viewUp * 2.8f + viewFront * 9.f;
    nodePos -= parent->getWorldPos();

    node.setPosition(nodePos);
    node.setRotation({-rot.x, 90 + rot.y, rot.z});

    return true;
}

void CameraController::onKey(Input* input, const ki::RenderClock& clock)
{
    if (!m_camera) return;

    float dt = clock.elapsedSecs;
    float moveSize = m_moveStep;
    float rotateSize = m_rotateStep;
    if (input->isModifierDown(Modifier::SHIFT)) {
        moveSize *= 2;
        rotateSize *= 2;
    }

    const auto& viewFront = m_camera->getViewFront();
    const auto& viewRight = m_camera->getViewRight();
    const auto& viewUp = m_camera->getViewUp();

    auto pos = m_camera->getPos();

    if (input->isKeyDown(Key::FORWARD)) {
        updateCamera();
        pos += viewFront * dt * moveSize;
    }

    if (input->isKeyDown(Key::BACKWARD)) {
        updateCamera();
        pos -= viewFront * dt * moveSize;
    }

    if (input->isKeyDown(Key::LEFT)) {
        updateCamera();
        pos -= viewRight * dt * moveSize;
    }

    if (input->isKeyDown(Key::RIGHT)) {
        updateCamera();
        pos += viewRight * dt * moveSize;
    }

    if (input->isKeyDown(Key::UP)) {
        updateCamera();
        pos += viewUp * dt * moveSize;
    }

    if (input->isKeyDown(Key::DOWN)) {
        updateCamera();
        pos -= viewUp * dt * moveSize;
    }

    m_camera->setPos(pos);

    if (true) {
        auto rotation = m_camera->getRotation();

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

    auto rotation = m_camera->getRotation();

    if (true) {
        rotation.y -= m_mouseSensitivity * xoffset;
        changed = true;
    }

    if (true) {
        auto pitch = rotation.x + m_mouseSensitivity * yoffset;
        changed = true;

        if (pitch < -MAX_ANGLE) {
            pitch = -MAX_ANGLE;
        }
        if (pitch > MAX_ANGLE) {
            pitch = MAX_ANGLE;
        }

        rotation.x = pitch;
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
