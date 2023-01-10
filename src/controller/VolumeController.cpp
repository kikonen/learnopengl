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
    if (!targetNode) return false;

    const auto& targetPos = targetNode->getWorldPos();

    auto radius = volumeNode.getVolume()->getRadius();

    const auto& volume = targetNode->getVolume();

    const auto& modelMatrix = targetNode->getModelMatrix();
    const glm::vec3 worldScale = {
        glm::length(modelMatrix[0]),
        glm::length(modelMatrix[1]),
        glm::length(modelMatrix[2]) };

    const auto& rootPos = ctx.m_nodeRegistry.m_root->getWorldPos();

    glm::vec3 volumePos{ modelMatrix * glm::vec4(volume->getCenter(), 1.f) };
    volumePos -= rootPos;
    const auto maxScale = std::max(std::max(worldScale.x, worldScale.y), worldScale.z);
    const auto volumeScale = volume->getRadius() * maxScale * 1.01f;

    volumeNode.setPosition(volumePos);
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
