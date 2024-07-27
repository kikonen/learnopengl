#include "FpsCamera.h"

#include "model/Node.h"

#include "render/Camera.h"

#include "render/RenderContext.h"

#include "registry/Registry.h"
#include "registry/NodeSnapshotRegistry.h"

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

    auto cameraPos = snapshot.getWorldPosition();
    const auto nodeFront = snapshot.getViewFront();
    const auto nodeUp = snapshot.getViewUp();

    //const auto& target = cameraPos + snapshot.getViewFront() * 100.0f;
    const glm::vec3 up{ 0, 0, 1 };

    //const auto& rot = util::normalToRotation(front, up);
    //auto degreesRot = util::quatToDegrees(rot);
    auto degreesRot = glm::vec3{ -15.f, 0.f, 0.f };

    cameraPos += -nodeFront * 3.f + nodeUp * 1.f;

    //m_camera.setNodeQuat(snapshot.getQuatRotation());

    m_camera.setDegreesRotation(degreesRot);
    m_camera.setWorldPosition(cameraPos);

    const auto& cameraFront = m_camera.getViewFront();

    m_nodeLevel = level;
}
