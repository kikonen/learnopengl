#include "FpsCamera.h"

#include "model/Node.h"

#include "render/Camera.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/NodeSnapshotRegistry.h"

namespace {
    const glm::vec3 UP{ 0, 1.f, 0 };
}

FpsCamera::FpsCamera() = default;
FpsCamera::~FpsCamera() = default;

void FpsCamera::updateRT(const UpdateContext& ctx, Node& node)
{
    if (!m_enabled) return;

    auto& snapshotRegistry = *ctx.m_registry->m_activeSnapshotRegistry;
    const auto& snapshot = snapshotRegistry.getSnapshot(node.m_snapshotIndex);

    const auto& level = snapshot.getMatrixLevel();
    const bool nodeChanged = m_nodeLevel != level;
    if (!nodeChanged) return;

    const auto& cameraPos = snapshot.getWorldPosition();
    const auto& cameraFront = snapshot.getViewFront();

    m_camera.setWorldPosition(cameraPos);
    m_camera.setAxis(cameraFront, UP);

    m_nodeLevel = level;
}

void FpsCamera::snapToIdeal(const Snapshot& snapshot)
{}
