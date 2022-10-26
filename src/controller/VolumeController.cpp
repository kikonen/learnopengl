#include "VolumeController.h"

#include "glm/glm.hpp"

#include "scene/RenderContext.h"
#include "scene/NodeRegistry.h"

VolumeController::VolumeController()
{
}

bool VolumeController::update(
    const RenderContext& ctx,
    Node& volumeNode,
    Node* parent)
{
    Node* targetNode = ctx.registry.getNode(m_targetID);
    if (!targetNode) return false;

    const auto& targetPos = targetNode->getWorldPos();

    auto radius = volumeNode.getVolume()->getRadius();

    const auto& volume = targetNode->getVolume();

    const auto& modelWorldMatrix = targetNode->getWorldModelMatrix();
    const glm::vec3 worldScale = {
        glm::length(modelWorldMatrix[0]),
        glm::length(modelWorldMatrix[1]),
        glm::length(modelWorldMatrix[2]) };

    const auto& rootPos = ctx.registry.m_root->getWorldPos();

    glm::vec3 volumePos{ modelWorldMatrix * glm::vec4(volume->getCenter(), 1.f) };
    volumePos -= rootPos;
    const auto maxScale = std::max(std::max(worldScale.x, worldScale.y), worldScale.z);
    const auto volumeScale = volume->getRadius() * maxScale * 1.01f;

    volumeNode.setPosition(volumePos);
    volumeNode.setScale(volumeScale);

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
