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

    glm::vec3 dir = glm::normalize(targetPos - cameraPos);

    const auto& viewMatrix = glm::lookAt(
        cameraPos,
        targetPos,
        glm::vec3{ 0, 1, 0 });
    const auto viewFront = glm::normalize(viewMatrix * glm::vec4{ 0, 0, 1, 1 });

    //auto degreesRot = glm::vec3{ 0.f, 0.f, 0.f };
    //m_camera.setDegreesRotation(degreesRot);
    m_camera.setWorldPosition(cameraPos);
    m_camera.setAxis(dir, snapshot.getViewUp());
    //m_camera.setAxis(dir, {0, 1, 0});

    const auto viewMatrix2 = m_camera.getView();
    const auto viewFront2 = glm::normalize(viewMatrix2 * glm::vec4{ 0, 0, 1, 1 });

    const auto& viewMatrix3 = glm::lookAt(
        cameraPos,
        targetPos,
        glm::vec3{ 0, 1, 0 });
    const auto viewFront3 = glm::normalize(viewMatrix * glm::vec4{ 0, 0, 1, 1 });

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
