#include "FollowCamera.h"

#include "model/Node.h"

#include "render/Camera.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/NodeSnapshotRegistry.h"

namespace {
    const glm::vec3 up{ 0, 1.f, 0 };
}

FollowCamera::FollowCamera() = default;
FollowCamera::~FollowCamera() = default;

void FollowCamera::updateRT(const UpdateContext& ctx, Node& node)
{
    if (!m_enabled) return;

    auto& snapshotRegistry = *ctx.m_registry->m_activeSnapshotRegistry;
    const auto& snapshot = snapshotRegistry.getSnapshot(node.m_snapshotIndex);

    const auto& level = snapshot.getMatrixLevel();
    const bool nodeChanged = m_nodeLevel != level;
    if (!nodeChanged) return;

    const auto& cameraPos = calculateCameraPos(snapshot);

    glm::vec3 targetPos = snapshot.getWorldPosition() +
        snapshot.getViewFront() * m_distance.z;

    glm::vec3 v = targetPos - cameraPos;

    auto degreesRot = glm::vec3{ -15.f, 0.f, 0.f };

    m_camera.setDegreesRotation(degreesRot);
    m_camera.setWorldPosition(cameraPos);

    const auto& camFront = m_camera.getViewFront();

    m_nodeLevel = level;
}

glm::vec3 FollowCamera::calculateCameraPos(const Snapshot& snapshot)
{
    glm::vec3 pos = snapshot.getWorldPosition();
    pos -= snapshot.getViewFront() * m_distance.x;
    pos += snapshot.getViewUp() * m_distance.y;
    return pos;
}
