#include "PawnController.h"

#include "util/glm_util.h"
#include "util/glm_format.h"

#include "gui/Input.h"

#include "model/Node.h"


#include "script/CommandEngine.h"
#include "script/api/RotateNode.h"
#include "script/api/MoveNode.h"

#include "engine/PrepareContext.h"
#include "engine/InputContext.h"

#include "registry/Registry.h"
#include "registry/SnapshotRegistry.h"


PawnController::PawnController()
{
}

void PawnController::prepare(
    const PrepareContext& ctx,
    Node& node)
{
    NodeController::prepare(ctx, node);

    auto& assets = ctx.m_assets;

    m_node = &node;

    m_speedMoveNormal = assets.cameraMoveNormal;
    m_speedMoveRun = assets.cameraMoveRun;
    m_speedRotateNormal = assets.cameraRotateNormal;
    m_speedRotateRun = assets.cameraRotateRun;

    m_speedMouseSensitivity = assets.cameraMouseSensitivity;
}

void PawnController::onKey(
    const InputContext& ctx)
{
    if (!m_node) return;

    const auto* input = ctx.m_input;

    const float dt = ctx.m_clock.elapsedSecs;

    const auto& snapshot = ctx.m_registry->m_snapshotRegistry->getActiveSnapshot(m_node->m_snapshotIndex);
    const auto& viewUp = snapshot.getViewUp();

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
                adjust.y += rotateSpeed.y * dt;
                changed = true;
            }
            if (input->isKeyDown(Key::ROTATE_RIGHT)) {
                adjust.y += -rotateSpeed.y * dt;
                changed = true;
            }

            if (changed) {
                m_registry->m_commandEngine->addCommand(
                    std::make_unique<script::RotateNode>(
                        0,
                        m_node->getId(),
                        0.f,
                        true,
                        snapshot.getViewUp(),
                        adjust.y));
                //m_node->getTransform().adjustQuatRotation(util::degreesToQuat(adjust));
            }
        }
    }

    {
        bool changed = false;
        glm::vec3 adjust{ 0.f };

        {
            const auto& viewFront = snapshot.getViewFront();

            if (input->isKeyDown(Key::FORWARD)) {
                adjust += viewFront * dt * moveSpeed.z;
                changed = true;
            }
            if (input->isKeyDown(Key::BACKWARD)) {
                adjust -= viewFront * dt * moveSpeed.z;
                changed = true;
            }
        }

        {
            const auto& viewRight = snapshot.getViewRight();

            if (input->isKeyDown(Key::LEFT)) {
                adjust -= viewRight * dt * moveSpeed.x;
                changed = true;
            }
            if (input->isKeyDown(Key::RIGHT)) {
                adjust += viewRight * dt * moveSpeed.x;
                changed = true;
            }
        }

        {
            if (input->isKeyDown(Key::UP)) {
                adjust += viewUp * dt * moveSpeed.y;
                changed = true;
            }
            if (input->isKeyDown(Key::DOWN)) {
                adjust -= viewUp * dt * moveSpeed.y;
                changed = true;
            }

        }

        if (changed) {
            m_registry->m_commandEngine->addCommand(
                std::make_unique<script::MoveNode>(
                    0,
                    m_node->getId(),
                    0.f,
                    true,
                    adjust));

            //glm::vec3 adjust = snapshot.getPosition();
            //m_node->getTransform().setPosition(pos);
        }
    }
}

void PawnController::onMouseMove(
    const InputContext& ctx,
    float xoffset,
    float yoffset)
{
    if (!m_node) return;

    bool changed = false;

    const auto& snapshot = ctx.m_registry->m_snapshotRegistry->getActiveSnapshot(m_node->m_snapshotIndex);

    glm::vec3 adjust{ 0.f };

    if (xoffset != 0) {
        auto yaw = -m_speedMouseSensitivity.x * xoffset;

        adjust.y = static_cast<float>(yaw);
        changed = true;
    }

    if (changed) {
        m_registry->m_commandEngine->addCommand(
            std::make_unique<script::RotateNode>(
                0,
                m_node->getId(),
                0.f,
                true,
                snapshot.getViewUp(),
                adjust.y));
        //m_node->getTransform().adjustQuatRotation(util::degreesToQuat(adjust));
    }
}
