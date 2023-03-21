#include "VolumeController.h"

#include "glm/glm.hpp"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

VolumeController::VolumeController()
{
}

bool VolumeController::update(
    const UpdateContext& ctx,
    Node& volumeNode) noexcept
{
    Node* targetNode = ctx.m_registry->m_nodeRegistry->getNode(m_targetID);

    if (!targetNode) {
        volumeNode.m_type->m_flags.noDisplay = true;
        return false;
    }

    const auto& modelMatrix = targetNode->getModelMatrix();
    const auto& maxScale = targetNode->getInstance().getWorldMaxScale();

    const auto& rootPos = ctx.m_registry->m_nodeRegistry->m_root->getWorldPosition();

    const auto& volume = targetNode->getVolume();
    const glm::vec3 volumeCenter = glm::vec3(volume);
    const float volumeRadius = volume.a;

    glm::vec3 pos{ modelMatrix * glm::vec4(volumeCenter, 1.f)};
    pos -= rootPos;

    const auto volumeScale = maxScale * volumeRadius;

    volumeNode.setPosition(pos);
    volumeNode.setScale(volumeScale);

    volumeNode.m_type->m_flags.noDisplay = false;

    return true;
}

int VolumeController::getTarget()
{
    return m_targetID;
}

void VolumeController::setTarget(int targetID)
{
    m_targetID = targetID;
}
