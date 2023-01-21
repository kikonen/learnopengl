#include "VolumeController.h"

#include "glm/glm.hpp"

#include "scene/RenderContext.h"
#include "registry/NodeRegistry.h"

VolumeController::VolumeController()
{
}

bool VolumeController::update(
    const RenderContext& ctx,
    Node& volumeNode,
    Node* parent) noexcept
{
    Node* targetNode = ctx.m_nodeRegistry.getNode(m_targetID);

    if (!targetNode) {
        volumeNode.m_type->m_flags.noDisplay = true;
        return false;
    }

    const auto& targetPos = targetNode->getWorldPosition();

    //auto radius = volumeNode.getVolume()->getRadius();

    //const auto& volume = targetNode->getVolume();

    const auto& modelMatrix = targetNode->getModelMatrix();
    const glm::vec3 worldScale = {
        glm::length(modelMatrix[0]),
        glm::length(modelMatrix[1]),
        glm::length(modelMatrix[2]) };

    const auto& rootPos = ctx.m_nodeRegistry.m_root->getPosition();
    const auto& volumeCenter = targetNode->getVolumeCenter();
    const auto volumeRadius = targetNode->getVolumeRadius();

    glm::vec3 pos{ modelMatrix * glm::vec4(volumeCenter, 1.f)};
    pos -= rootPos;

    const auto maxScale = std::max(std::max(worldScale.x, worldScale.y), worldScale.z);
    //const auto volumeScale = volume->getRadius() * maxScale * 1.01f;
    glm::vec3 radiusPos = modelMatrix *
        glm::vec4(volumeCenter + glm::vec3(volumeRadius, 0, 0), 1.f);
    radiusPos -= rootPos;

    const auto volumeScale = glm::length(radiusPos - pos);
    const auto volumeScale2 = maxScale * volumeRadius;

    volumeNode.setPosition(pos);
    //volumeNode.setScale(volumeScale);
    volumeNode.setScale(volumeScale2);

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
