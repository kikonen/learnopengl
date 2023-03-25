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

    // NOTE KI need to keep "ratio" between these similar speed wise
    m_forwardStep = 5.5f;
    m_upStep = 6.5f;
    m_rightStep = 4.5f;

    m_rotateStep = 12.5f;

    m_zoomStep = 20.0f;
    m_mouseSensitivity = 0.1f;
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
    float forwardSize = m_forwardStep;
    float upSize = m_upStep;
    float rightSize = m_rightStep;
    float rotateSize = m_rotateStep;

    if (input->isModifierDown(Modifier::SHIFT)) {
        // NOTE KI need to keep "ratio" between these similar speed wise
        forwardSize *= 3;
        upSize *= 3;
        rightSize *= 2;
        rotateSize *= 2.5;
    }

    {
        if (true) {
            glm::vec3 rotation = m_node->getRotation();

            if (input->isKeyDown(Key::ROTATE_LEFT)) {
                rotation.y += rotateSize * dt;
            }
            if (input->isKeyDown(Key::ROTATE_RIGHT)) {
                rotation.y -= rotateSize * dt;
            }

            m_node->setRotation({ rotation.x, rotation.y, rotation.z });
        }

        if (input->isKeyDown(Key::ZOOM_IN)) {
            camera->adjustZoom(-m_zoomStep * dt);
        }
        if (input->isKeyDown(Key::ZOOM_OUT)) {
            camera->adjustZoom(m_zoomStep * dt);
        }
    }

    {
        bool changed = false;
        glm::vec3 pos = m_node->getPosition();

        {
            const auto& viewFront = camera->getViewFront();

            if (input->isKeyDown(Key::FORWARD)) {
                pos += viewFront * dt * forwardSize;
                changed = true;
            }
            if (input->isKeyDown(Key::BACKWARD)) {
                pos -= viewFront * dt * forwardSize;
                changed = true;
            }
        }

        {
            const auto& viewRight = camera->getViewRight();

            if (input->isKeyDown(Key::LEFT)) {
                pos -= viewRight * dt * rightSize;
                changed = true;
            }
            if (input->isKeyDown(Key::RIGHT)) {
                pos += viewRight * dt * rightSize;
                changed = true;
            }
        }

        {
            const auto& viewUp = camera->getViewUp();

            if (input->isKeyDown(Key::UP)) {
                pos += viewUp * dt * upSize;
                changed = true;
            }
            if (input->isKeyDown(Key::DOWN)) {
                pos -= viewUp * dt * upSize;
                changed = true;
            }

        }

        if (changed) {
            m_node->setPosition(pos);
        }
    }
}

void CameraController::onMouseMove(Input* input, double xoffset, double yoffset)
{
    if (!m_node) return;

    bool changed = false;
    const float MAX_ANGLE = 89.f;

    glm::vec3 rotation = m_node->getRotation();

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
        m_node->setRotation(rotation);
    }
}

void CameraController::onMouseScroll(Input* input, double xoffset, double yoffset)
{
    if (!m_node) return;
    auto* camera = m_node->m_camera.get();

    camera->adjustZoom(-yoffset);
}
