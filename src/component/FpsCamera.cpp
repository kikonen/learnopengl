#include "FpsCamera.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "render/Camera.h"

#include "render/RenderContext.h"


namespace {
    const glm::vec3 UP{ 0, 1.f, 0 };
}

FpsCamera::FpsCamera() = default;
FpsCamera::~FpsCamera() = default;

void FpsCamera::updateRT(const UpdateContext& ctx, model::Node& node)
{
    if (!m_enabled) return;

    const auto* snapshot = node.getSnapshotRT();
    if (!snapshot) return;

    const auto dt = ctx.m_clock.elapsedSecs;

    const auto& level = snapshot->getMatrixLevel();
    const bool nodeChanged = m_nodeLevel != level;
    if (!nodeChanged) return;

    // Update pitch based on pitch speed
    m_pitch += m_pitchSpeed * dt;

    // Clamp pitch to [-max, +max]
    m_pitch = std::min(std::max(m_pitch, -m_maxPitch), m_maxPitch);

    // Make a quaternion representing pitch rotation,
    // which is about owner's right vector
    glm::quat q = util::axisRadiansToQuat(snapshot->getViewRight(), m_pitch);

    const auto& cameraPos = snapshot->getWorldPosition();
    const auto& cameraFront = q * snapshot->getViewFront();

    m_camera.setWorldPosition(cameraPos);
    m_camera.setAxis(cameraFront, UP);

    m_nodeLevel = level;
}

void FpsCamera::snapToIdeal(const Snapshot& snapshot)
{}
