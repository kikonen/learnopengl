#include "OrbitCamera.h"

#include <glm/gtx/quaternion.hpp>

#include "util/debug.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "render/Camera.h"

#include "render/RenderContext.h"

namespace {
    const glm::vec3 UP{ 0, 1.f, 0 };
}

OrbitCamera::OrbitCamera() = default;
OrbitCamera::~OrbitCamera() = default;

void OrbitCamera::updateRT(const UpdateContext& ctx, Node& node)
{
    if (!m_enabled) return;

    const auto dt = ctx.m_clock.elapsedSecs;
    const auto& snapshot = node.getActiveSnapshot(ctx.m_registry);

    {
        // Create a quaternion for yaw about world up
        glm::quat yaw = util::axisRadiansToQuat(UP, m_yawSpeed * dt);

        // Transform offset and up by yaw
        m_offset = yaw * m_offset;
        m_up = yaw * m_up;
    }
    {
        // Compute camera forward/right from these vectors
        // Forward owner.position - (owner.position + offset)
        // = -offset
        glm::vec3 front = glm::normalize(- 1.f * m_offset);

        glm::vec3 right = glm::normalize(glm::cross(m_up, front));

        // Create quaternion for pitch about camera right
        glm::quat pitch = util::axisRadiansToQuat(right, m_pitchSpeed * dt);

        // Transform camera offset and up by pitch
        m_offset = pitch * m_offset, pitch;
        m_up = pitch * m_up, pitch;
    }

    // Compute transform matrix
    glm::vec3 target = snapshot.getWorldPosition();

    const glm::vec3 cameraPos = target + m_offset;
    const glm::vec3 cameraFront = glm::normalize(target - cameraPos);

    m_camera.setWorldPosition(cameraPos);
    m_camera.setAxis(cameraFront, m_up);
}
