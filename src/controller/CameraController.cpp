#include "CameraController.h"


CameraController::CameraController()
{
}

bool CameraController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent) noexcept
{
    Camera* camera = node.m_camera.get();
    if (!camera) return false;

    glm::vec3 viewFront = camera->getViewFront();
    glm::vec3 viewUp = camera->getViewUp();
    glm::vec3 pos = camera->getPos();// +(front * 0.1f);
    glm::vec3 rot = camera->getRotation();

    auto nodePos = pos - viewUp * 2.8f + viewFront * 9.f;
    nodePos -= parent->getWorldPos();

    node.setPosition(nodePos);
    node.setRotation({-rot.x, 90 + rot.y, rot.z});

    return true;
}
