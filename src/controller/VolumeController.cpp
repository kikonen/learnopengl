#include "VolumeController.h"

#include "glm/glm.hpp"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

VolumeController::VolumeController()
{
}

bool VolumeController::updateWT(
    const UpdateContext& ctx,
    Node& volumeNode) noexcept
{
    if (!m_targetID) return false;

    Node* targetNode = ctx.m_registry->m_nodeRegistry->getNode(m_targetID);

    if (!targetNode) {
        volumeNode.m_visible = false;
        return false;
    }

    const auto& transform = targetNode->getTransform();
    const auto& modelMatrix = transform.getModelMatrix();
    const auto& maxScale = transform.getWorldMaxScale();

    const auto& rootPos = ctx.m_registry->m_nodeRegistry->m_root->getSnapshot().getWorldPosition();

    const auto& volume = transform.getVolume();
    const glm::vec3 volumeCenter = glm::vec3(volume);
    const float volumeRadius = volume.a;

    glm::vec3 pos{ modelMatrix * glm::vec4(volumeCenter, 1.f)};
    pos -= rootPos;
    const auto volumeScale = maxScale * volumeRadius;

    {
        auto& volumeTransform = volumeNode.modifyTransform();
        volumeTransform.setFront(transform.getFront());
        volumeTransform.setQuatRotation(transform.getQuatRotation());
        volumeTransform.setPosition(pos);
        volumeTransform.setScale(volumeScale);

        volumeNode.m_visible = true;
    }

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
