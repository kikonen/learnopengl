#include "PawnController.h"

#include "util/glm_util.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"


PawnController::PawnController()
{
}

void PawnController::prepare(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    m_node = &node;

    m_speedMoveNormal = assets.cameraMoveNormal;
    m_speedMoveRun = assets.cameraMoveRun;
    m_speedRotateNormal = assets.cameraRotateNormal;
    m_speedRotateRun = assets.cameraRotateRun;

    m_speedMouseSensitivity = assets.cameraMouseSensitivity;
}

bool PawnController::update(
    const UpdateContext& ctx,
    Node& node) noexcept
{
    return false;
}

void PawnController::onKey(Input* input, const ki::RenderClock& clock)
{
    if (!m_node) return;

    const float dt = clock.elapsedSecs;

    glm::vec3 moveSpeed{ m_speedMoveNormal };
    glm::vec3 rotateSpeed{ m_speedRotateNormal };

    if (input->isModifierDown(Modifier::SHIFT)) {
        moveSpeed = m_speedMoveRun;
        rotateSpeed = m_speedRotateRun;

        if (input->isModifierDown(Modifier::ALT)) {
            moveSpeed *= 10;
            rotateSpeed *= 2;
        }
    }

    {
        if (true) {
            bool changed = false;
            glm::vec3 adjust{ 0.f };

            if (input->isKeyDown(Key::ROTATE_LEFT)) {
                adjust.y = rotateSpeed.y * dt;
                changed = true;
            }
            if (input->isKeyDown(Key::ROTATE_RIGHT)) {
                adjust.y = -rotateSpeed.y * dt;
                changed = true;
            }

            if (changed) {
                m_node->adjustQuatRotation(util::degreesToQuat(adjust));
            }
        }
    }

    {
        bool changed = false;
        glm::vec3 pos = m_node->getPosition();

        {
            const auto& viewFront = m_node->getViewFront();

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
            const auto& viewRight = m_node->getViewRight();

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
            const auto& viewUp = m_node->getViewUp();

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

void PawnController::onMouseMove(Input* input, float xoffset, float yoffset)
{
    if (!m_node) return;

    bool changed = false;
    const float MAX_ANGLE = 89.f;

    glm::vec3 rot = m_node->getDegreesRotation();

    if (xoffset != 0) {
        auto yaw = rot.y - m_speedMouseSensitivity.x * xoffset;

        rot.y = static_cast<float>(yaw);
        changed = true;
    }

    if (yoffset != 0) {
        auto pitch = rot.x + m_speedMouseSensitivity.y * yoffset;

        if (pitch < -MAX_ANGLE) {
            pitch = -MAX_ANGLE;
        }
        if (pitch > MAX_ANGLE) {
            pitch = MAX_ANGLE;
        }

        rot.x = static_cast<float>(pitch);
        changed = true;
    }

    if (changed) {
        m_node->setDegreesRotation(rot);
    }
}
