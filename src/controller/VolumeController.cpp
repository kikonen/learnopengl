#include "VolumeController.h"

#include "glm/glm.hpp"

#include "pool/NodeHandle.h"

#include "model/Node.h"

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
    Node* targetNode = m_targetId.toNode();

    if (!targetNode) {
        volumeNode.m_visible = false;
        return false;
    }

    const auto& state = targetNode->getState();
    const auto& modelMatrix = state.getModelMatrix();
    const auto& maxScale = state.getWorldMaxScale();

    const auto* rootNode = NodeRegistry::get().getRootWT();
    const auto& rootPos = rootNode->getState().getWorldPosition();

    const auto& volume = state.getVolume();
    const glm::vec3 volumeCenter = glm::vec3(volume);
    const float volumeRadius = volume.a;

    glm::vec3 pos{ modelMatrix * glm::vec4(volumeCenter, 1.f)};
    pos -= rootPos;
    const auto volumeScale = maxScale * volumeRadius;

    {
        auto& volumeState = volumeNode.modifyState();
        volumeState.setFront(state.getFront());
        volumeState.setQuatRotation(state.getQuatRotation());
        volumeState.setPosition(pos);
        volumeState.setScale(volumeScale);

        volumeNode.m_visible = true;
    }

    return true;
}
