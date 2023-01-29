#include "CameraController.h"

#include "model/Node.h"

#include "scene/RenderContext.h"
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
    if (!m_node) return false;
    auto* camera = m_node->m_camera.get();

    const auto& viewFront = camera->getViewFront();
    const auto& viewUp = camera->getViewUp();
    //const auto& pos = camera->getPos();// +(front * 0.1f);
    const auto& rot = camera->getRotation();

    //auto nodePos = pos - viewUp * 2.8f + viewFront * 9.f;
    //nodePos -= parent->getWorldPosition();

    //node.setPosition(nodePos);
    node.setRotation({-rot.x, 90 + rot.y, rot.z});

    return true;
}

void CameraController::onKey(Input* input, const ki::RenderClock& clock)
{
    if (!m_node) return;
    auto* camera = m_node->m_camera.get();

    const float dt = clock.elapsedSecs;
    float moveSize = m_moveStep;
    float rotateSize = m_rotateStep;
    if (input->isModifierDown(Modifier::SHIFT)) {
        moveSize *= 3;
        rotateSize *= 3;
    }

    const auto& viewFront = camera->getViewFront();
    const auto& viewRight = camera->getViewRight();
    const auto& viewUp = camera->getViewUp();

    glm::vec3 pos = m_node->getPosition();

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

    m_node->setPosition(pos);

    if (true) {
        glm::vec3 rotation = camera->getRotation();

        if (input->isKeyDown(Key::ROTATE_LEFT)) {
            rotation.y  += rotateSize * dt;
        }
        if (input->isKeyDown(Key::ROTATE_RIGHT)) {
            rotation.y -= rotateSize * dt;
        }

        camera->setRotation(rotation);
    }


    if (input->isKeyDown(Key::ZOOM_IN)) {
        camera->adjustZoom(-m_zoomStep * dt);
    }
    if (input->isKeyDown(Key::ZOOM_OUT)) {
        camera->adjustZoom(m_zoomStep * dt);
    }
}

void CameraController::onMouseMove(Input* input, double xoffset, double yoffset)
{
    if (!m_node) return;
    auto* camera = m_node->m_camera.get();

    bool changed = false;
    const float MAX_ANGLE = 89.f;

    glm::vec3 rotation = camera->getRotation();

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
        camera->setRotation(rotation);
    }
}

void CameraController::onMouseScroll(Input* input, double xoffset, double yoffset)
{
    if (!m_node) return;
    auto* camera = m_node->m_camera.get();

    camera->adjustZoom(-yoffset);
}
