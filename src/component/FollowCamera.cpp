#include "FollowCamera.h"

#include "util/debug.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "render/Camera.h"

#include "render/RenderContext.h"

namespace {
    const glm::vec3 UP{ 0, 1.f, 0 };
}

FollowCamera::FollowCamera() = default;
FollowCamera::~FollowCamera() = default;

// Game Programming in C++ - chapter 9
void FollowCamera::updateRT(const UpdateContext& ctx, Node& node)
{
    if (!m_enabled) return;

    const auto dt = ctx.m_clock.elapsedSecs;
    const auto& snapshot = node.getActiveSnapshot(ctx.m_registry);

    //const auto& level = snapshot.getMatrixLevel();
    //const bool nodeChanged = m_nodeLevel != level;
    //if (!nodeChanged) return;

    // Compute dampening from spring constant
    const float dampening = 2.0f * std::sqrt(m_springConstant);

    // Compute ideal position
    const auto& idealPos = calculateCameraPos(snapshot);

    // Compute acceleration of spring
    const auto diff = m_actualPos - idealPos;
    const auto acel = -m_springConstant * diff -
        dampening * m_velocity;

    // Update velocity
    m_velocity += acel * dt;

    // Update actual camera position
    m_actualPos += m_velocity * dt;

    glm::vec3 target = snapshot.getWorldPosition() +
        snapshot.getViewFront() * m_distance.z;

    glm::vec3 cameraFront = glm::normalize(target - m_actualPos);

    m_camera.setWorldPosition(m_actualPos);
    m_camera.setAxis(cameraFront, UP);

    //m_nodeLevel = level;
}

void FollowCamera::snapToIdeal(const Snapshot& snapshot)
{
    // Set actual position to ideal
    m_actualPos = calculateCameraPos(snapshot);

    // Zero velocity
    m_velocity = glm::vec3{ 0.f };

    // Compute target and view
    glm::vec3 target = snapshot.getWorldPosition() +
        snapshot.getViewFront() * m_distance.z;

    glm::vec3 cameraFront = glm::normalize(target - m_actualPos);

    m_camera.setWorldPosition(m_actualPos);
    m_camera.setAxis(cameraFront, UP);
}

glm::vec3 FollowCamera::calculateCameraPos(const Snapshot& snapshot)
{
    glm::vec3 pos = snapshot.getWorldPosition();
    pos -= snapshot.getViewFront() * m_distance.x;
    pos += snapshot.getViewUp() * m_distance.y;
    return pos;
}
